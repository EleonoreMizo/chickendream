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
#if ! defined (chkdravs_Grain_HEADER_INCLUDED)
#define chkdravs_Grain_HEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "avsutl/PlaneProcCbInterface.h"
#include "avsutl/PlaneProcessor.h"
#include "avsutl/VideoFilterBase.h"
#include "chkdr/CpuOptBase.h"
#include "chkdr/GrainProc.h"

#include <memory>
#include <mutex>
#include <vector>



namespace chkdravs
{



class Grain
:	public avsutl::VideoFilterBase
,	public avsutl::PlaneProcCbInterface
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	typedef avsutl::VideoFilterBase Inherited;

	enum Param
	{
		Param_CLIP_SRC = 0, // 0
		Param_SIGMA,
		Param_RES,
		Param_RAD,
		Param_DEV,
		Param_SEED,
		Param_CF,
		Param_CP,
		Param_CPUOPT,

		Param_NBR_ELT,
	};

	explicit       Grain (::IScriptEnvironment &env, const ::AVSValue &args);
	virtual        ~Grain () = default;

	// VideoFilterBase
	::PVideoFrame __stdcall
						GetFrame (int n, ::IScriptEnvironment *env_ptr) override;



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:

	// PlaneProcCbInterface
	void           do_process_plane (::PVideoFrame &dst_sptr, int n, ::IScriptEnvironment &env, int plane_index, int plane_id, void *ctx_ptr) override;



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	class CpuOpt : public chkdr::CpuOptBase
	{
	public:
		explicit       CpuOpt (const ::AVSValue &arg);
	};

	::PClip        _clip_src_sptr;
	const ::VideoInfo
	               _vi_src;

	std::unique_ptr <avsutl::PlaneProcessor>
	               _plane_proc_uptr;

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



}  // namespace chkdravs



//#include "chkdravs/Grain.hpp"



#endif   // chkdravs_Grain_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
