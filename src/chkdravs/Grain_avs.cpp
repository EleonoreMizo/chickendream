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

#include "avsutl/fnc.h"
#include "chkdravs/function_names.h"
#include "chkdravs/Grain.h"
#include "fstb/def.h"

#include <cassert>



namespace chkdravs
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



Grain::Grain (::IScriptEnvironment &env, const ::AVSValue &args)
:	Inherited (env, args [Param_CLIP_SRC].AsClip ())
,	_clip_src_sptr (args [Param_CLIP_SRC].AsClip ())
,	_vi_src (vi)
{
	const CpuOpt   cpu_opt (args [Param_CPUOPT]);
	const bool     simd4_flag = cpu_opt.has_sse2 ();
	const bool     avx_flag   = cpu_opt.has_avx ();

	if (! _vi_src.IsPlanar ())
	{
		env.ThrowError (chkdravs_GRAIN ": input must be planar.");
	}

	if (! (avsutl::is_rgb (vi) || vi.IsY ()))
	{
		env.ThrowError (chkdravs_GRAIN ": only linear RGB and Y colorformats are supported.");
	}

	if (vi.BitsPerComponent () != 32)
	{
		env.ThrowError (chkdravs_GRAIN ": only 32-bit float data type is supported.");
	}

	const auto     sigma   = float (args [Param_SIGMA].AsFloat (0.35f));
	const auto     res     =        args [Param_RES  ].AsInt (1000);
	const auto     rad     = float (args [Param_RAD  ].AsFloat (0.025f));
	const auto     dev     = float (args [Param_DEV  ].AsFloat (0));
	const auto     seed    = uint32_t (args [Param_SEED ].AsInt (12345));
	const auto     cf_flag = args [Param_CF].AsBool (false);
	const auto     cp_flag = args [Param_CP].AsBool (false);

	if (! chkdr::GrainProc::check_sigma (sigma))
	{
		env.ThrowError (chkdravs_GRAIN ": sigma must be in range [0 ; 1].");
	}
	if (! chkdr::GrainProc::check_res (res))
	{
		env.ThrowError (chkdravs_GRAIN ": res must be > 0.");
	}
	if (! chkdr::GrainProc::check_rad (rad))
	{
		env.ThrowError (chkdravs_GRAIN ": rad must be > 0.");
	}
	if (! chkdr::GrainProc::check_dev (dev))
	{
		env.ThrowError (chkdravs_GRAIN ": dev must be in range [0 ; 1]");
	}

	// Configures the plane processor
	_plane_proc_uptr =
		std::make_unique <avsutl::PlaneProcessor> (vi, *this, false);
	_plane_proc_uptr->set_clip_info (
		avsutl::PlaneProcessor::ClipIdx_SRC1, _clip_src_sptr
	);
	_plane_proc_uptr->set_proc_mode ("all");

	_proc_uptr = std::make_unique <chkdr::GrainProc> (
		sigma, res, rad, dev, seed, cf_flag, cp_flag, simd4_flag, avx_flag
	);
}



::PVideoFrame __stdcall	Grain::GetFrame (int n, ::IScriptEnvironment *env_ptr)
{
	::PVideoFrame  src_sptr = _clip_src_sptr->GetFrame (n, env_ptr);
	::PVideoFrame	dst_sptr = build_new_frame (*env_ptr, vi, &src_sptr);

	_plane_proc_uptr->process_frame (dst_sptr, n, *env_ptr, nullptr);

	return dst_sptr;
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



void	Grain::do_process_plane (::PVideoFrame &dst_sptr, int n, ::IScriptEnvironment &env, int plane_index, int plane_id, void *ctx_ptr)
{
	fstb::unused (ctx_ptr);

	::PVideoFrame  src_sptr     = _clip_src_sptr->GetFrame (n, &env);

	uint8_t *      data_dst_ptr = dst_sptr->GetWritePtr (plane_id);
	const int      stride_dst   = dst_sptr->GetPitch (plane_id);
	const uint8_t* data_src_ptr = src_sptr->GetReadPtr (plane_id);
	const int      stride_src   = src_sptr->GetPitch (plane_id);
	const int      w = _plane_proc_uptr->get_width (
		dst_sptr, plane_id, avsutl::PlaneProcessor::ClipIdx_DST
	);
	const int      h = _plane_proc_uptr->get_height (dst_sptr, plane_id);

	try
	{
		_proc_uptr->process_plane (
			data_dst_ptr, stride_dst,
			data_src_ptr, stride_src,
			w, h,
			n, plane_index
		);
	}

	catch (...)
	{
		assert (false);
	}
}



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



Grain::CpuOpt::CpuOpt (const ::AVSValue &arg)
{
	set_level (static_cast <Level> (
		arg.AsInt (Level_ANY_AVAILABLE) & Level_MASK
	));
}



}  // namespace chkdravs



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
