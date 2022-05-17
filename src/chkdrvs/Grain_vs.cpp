/*****************************************************************************

        Grain.cpp
        Author: Laurent de Soras, 2022

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/




/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "chkdrvs/Grain.h"
#include "fstb/def.h"
#include "vsutl/fnc.h"

#include <cassert>



namespace chkdrvs
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



Grain::Grain (const ::VSMap &in, ::VSMap &out, void *user_data_ptr, ::VSCore &core, const ::VSAPI &vsapi)
:	vsutl::FilterBase (vsapi, "grain", ::fmParallel)
,	_clip_src_sptr (vsapi.mapGetNode (&in, "clip", 0, nullptr), vsapi)
,	_vi_in (*_vsapi.getVideoInfo (_clip_src_sptr.get ()))
,	_vi_out (_vi_in)
#if defined (_MSC_VER)
#pragma warning (push)
#pragma warning (disable : 4355)
#endif // 'this': used in base member initializer list
,	_plane_processor (vsapi, *this, "grain", true)
#if defined (_MSC_VER)
#pragma warning (pop)
#endif
{
	fstb::unused (user_data_ptr, core);

	const CpuOpt   cpu_opt (*this, in, out);
	const bool     simd4_flag = cpu_opt.has_sse2 ();
	const bool     avx_flag   = cpu_opt.has_avx ();

	// Checks the input clip
	if (! vsutl::is_constant_format (_vi_in))
	{
		throw_inval_arg ("only constant pixel formats are supported.");
	}

	// Source colorspace
	const auto &   fmt_src = _vi_in.format;
	if (fmt_src.bitsPerSample != 32)
	{
		throw_inval_arg ("only 32-bit float data type is supported.");
	}
	if (fmt_src.colorFamily != ::cfGray && fmt_src.colorFamily != ::cfRGB)
	{
		throw_inval_arg ("only linear RGB and Y colorformats are supported.");
	}

	_plane_processor.set_filter (in, out, _vi_out, true);

	const auto     sigma   = float (get_arg_flt (in, out, "sigma", 0.35f));
	const auto     res     =        get_arg_int (in, out, "res"  , 1024);
	const auto     rad     = float (get_arg_flt (in, out, "rad"  , 0.025f));
	const auto     dev     = float (get_arg_flt (in, out, "dev"  , 0));
	const auto     seed    = uint32_t (get_arg_int (in, out, "seed" , 12345));
	const auto     cf_flag = (get_arg_int (in, out, "cf", 0) != 0);
	const auto     cp_flag = (get_arg_int (in, out, "cp", 0) != 0);
	const auto     draft_flag = (get_arg_int (in, out, "draft", 0) != 0);

	if (! chkdr::GrainProc::check_sigma (sigma))
	{
		throw_inval_arg (": sigma must be in range [0 ; 1].");
	}
	if (! chkdr::GrainProc::check_res (res))
	{
		throw_inval_arg (": res must be > 0.");
	}
	if (! chkdr::GrainProc::check_rad (rad))
	{
		throw_inval_arg (": rad must be > 0.");
	}
	if (! chkdr::GrainProc::check_dev (dev))
	{
		throw_inval_arg (": dev must be in range [0 ; 1]");
	}

	_proc_uptr = std::make_unique <chkdr::GrainProc> (
		sigma, res, rad, dev, seed, cf_flag, cp_flag, draft_flag,
		simd4_flag, avx_flag
	);
}



::VSVideoInfo	Grain::get_video_info () const
{
	return _vi_out;
}



std::vector <::VSFilterDependency>	Grain::get_dependencies () const
{
	return std::vector <::VSFilterDependency> {
		{ &*_clip_src_sptr, ::rpStrictSpatial }
	};
}



const ::VSFrame *	Grain::get_frame (int n, int activation_reason, void * &frame_data_ptr, ::VSFrameContext &frame_ctx, ::VSCore &core)
{
	assert (n >= 0);

	::VSFrame *    dst_ptr = nullptr;
	::VSNode &     node    = *_clip_src_sptr;

	if (activation_reason == ::arInitial)
	{
		_vsapi.requestFrameFilter (n, &node, &frame_ctx);
	}

	else if (activation_reason == ::arAllFramesReady)
	{
		vsutl::FrameRefSPtr	src_sptr (
			_vsapi.getFrameFilter (n, &node, &frame_ctx),
			_vsapi
		);
		const ::VSFrame & src = *src_sptr;

		const int      w = _vsapi.getFrameWidth (&src, 0);
		const int      h = _vsapi.getFrameHeight (&src, 0);
		dst_ptr = _vsapi.newVideoFrame (&_vi_out.format, w, h, &src, &core);

		const int      ret_val = _plane_processor.process_frame (
			*dst_ptr, n, frame_data_ptr, frame_ctx, core, _clip_src_sptr
		);
		if (ret_val != 0)
		{
			_vsapi.freeFrame (dst_ptr);
			dst_ptr = nullptr;
		}
	}

	return dst_ptr;
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



int	Grain::do_process_plane (::VSFrame &dst, int n, int plane_index, void *frame_data_ptr, ::VSFrameContext &frame_ctx, ::VSCore &core, const vsutl::NodeRefSPtr &src_node1_sptr, const vsutl::NodeRefSPtr &src_node2_sptr, const vsutl::NodeRefSPtr &src_node3_sptr)
{
	fstb::unused (frame_data_ptr, core, src_node2_sptr, src_node3_sptr);
	assert (src_node1_sptr.get () != nullptr);

	int            ret_val = 0;

	const vsutl::PlaneProcMode proc_mode =
		_plane_processor.get_mode (plane_index);

	if (proc_mode == vsutl::PlaneProcMode_PROCESS)
	{
		vsutl::FrameRefSPtr	src_sptr (
			_vsapi.getFrameFilter (n, src_node1_sptr.get (), &frame_ctx),
			_vsapi
		);
		const ::VSFrame & src = *src_sptr;

		const int      w = _vsapi.getFrameWidth (&src, plane_index);
		const int      h = _vsapi.getFrameHeight (&src, plane_index);

		const uint8_t* data_src_ptr = _vsapi.getReadPtr (&src, plane_index);
		const auto     stride_src   = _vsapi.getStride (&src, plane_index);
		uint8_t *      data_dst_ptr = _vsapi.getWritePtr (&dst, plane_index);
		const auto     stride_dst   = _vsapi.getStride (&dst, plane_index);

		try
		{
			_proc_uptr->process_plane (
				data_dst_ptr, stride_dst,
				data_src_ptr, stride_src,
				w, h,
				n, plane_index
			);
		}

		catch (std::exception &e)
		{
			_vsapi.setFilterError (e.what (), &frame_ctx);
			ret_val = -1;
		}
		catch (...)
		{
			_vsapi.setFilterError ("grain: exception.", &frame_ctx);
			ret_val = -1;
		}
	}

	return ret_val;
}



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



Grain::CpuOpt::CpuOpt (vsutl::FilterBase &filter, const ::VSMap &in, ::VSMap &out, const char *param_name_0)
{
	assert (param_name_0 != 0);
	set_level (static_cast <Level> (filter.get_arg_int (
		in, out, param_name_0, Level_ANY_AVAILABLE
	) & Level_MASK));
}



}  // namespace chkdrvs



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
