/*****************************************************************************

        Grain.h
        Author: Laurent de Soras, 2022

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (chkdrvs_Grain_HEADER_INCLUDED)
#define chkdrvs_Grain_HEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "chkdr/GrainProc.h"
#include "chkdr/CpuOptBase.h"
#include "vsutl/FilterBase.h"
#include "vsutl/NodeRefSPtr.h"
#include "vsutl/PlaneProcCbInterface.h"
#include "vsutl/PlaneProcessor.h"
#include "VapourSynth4.h"
#include "avstp.h"

#include <memory>
#include <mutex>
#include <vector>



namespace chkdrvs
{



class Grain
:	public vsutl::FilterBase
,	public vsutl::PlaneProcCbInterface
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	typedef	Grain	ThisType;

	explicit       Grain (const ::VSMap &in, ::VSMap &out, void *user_data_ptr, ::VSCore &core, const ::VSAPI &vsapi);
	virtual        ~Grain () {}

	// vsutl::FilterBase
	virtual ::VSVideoInfo
	               get_video_info () const;
	virtual std::vector <::VSFilterDependency>
	               get_dependencies () const;
	virtual const ::VSFrame *
	               get_frame (int n, int activation_reason, void * &frame_data_ptr, ::VSFrameContext &frame_ctx, ::VSCore &core);



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:

	// vsutl::PlaneProcCbInterface
	virtual int    do_process_plane (::VSFrame &dst, int n, int plane_index, void *frame_data_ptr, ::VSFrameContext &frame_ctx, ::VSCore &core, const vsutl::NodeRefSPtr &src_node1_sptr, const vsutl::NodeRefSPtr &src_node2_sptr, const vsutl::NodeRefSPtr &src_node3_sptr);



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	class CpuOpt : public chkdr::CpuOptBase
	{
	public:
		explicit       CpuOpt (vsutl::FilterBase &filter, const ::VSMap &in, ::VSMap &out, const char *param_name_0 = "cpuopt");
	};

	vsutl::NodeRefSPtr
	               _clip_src_sptr;
	const ::VSVideoInfo             
	               _vi_in;        // Input. Must be declared after _clip_src_sptr because of initialisation order.
	::VSVideoInfo  _vi_out;       // Output. Must be declared after _vi_in.

	vsutl::PlaneProcessor
	               _plane_processor;

	std::unique_ptr <chkdr::GrainProc>
	               _proc_uptr;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	               Grain ()                               = delete;
	               Grain (const Grain &other)             = delete;
	               Grain (Grain &&other)                  = delete;
	Grain &        operator = (const Grain &other)        = delete;
	Grain &        operator = (Grain &&other)             = delete;
	bool           operator == (const Grain &other) const = delete;
	bool           operator != (const Grain &other) const = delete;

}; // class Grain



}  // namespace chkdrvs



//#include "chkdrvs/Grain.hpp"



#endif   // chkdrvs_Grain_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
