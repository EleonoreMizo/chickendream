/*****************************************************************************

        GrainProc.h
        Author: Laurent de Soras, 2022

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (chkdr_GrainProc_HEADER_INCLUDED)
#define chkdr_GrainProc_HEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "chkdr/AvstpScopedDispatcher.h"
#include "fgrn/GenGrain.h"
#include "fgrn/VisionFilter.h"
#include "avstp.h"

#include <memory>
#include <mutex>
#include <vector>



class AvstpWrapper;

namespace chkdr
{



class GrainProc
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	explicit       GrainProc (float sigma, int res, float rad, float dev, uint32_t seed, bool cf_flag, bool cp_flag, bool draft_flag, bool simd4_flag, bool avx_flag);
	virtual        ~GrainProc () {}

	void           process_plane (uint8_t *dst_ptr, ptrdiff_t dst_stride, const uint8_t *src_ptr, ptrdiff_t src_stride, int w, int h, int frame_idx, int plane_idx);

	static bool    check_sigma (float sigma) noexcept;
	static bool    check_res (int res) noexcept;
	static bool    check_rad (float rad) noexcept;
	static bool    check_dev (float dev) noexcept;



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	class TaskInfo
	{
	public:
		fgrn::GenGrain *
		               _gen_ptr = nullptr;
		int            _pass    = 0; // 1 or 2
		int            _tid     = 0; // Task identifier
	};

	class FrameProc
	{
	public:
		explicit       FrameProc (bool simd4_flag, bool avx_flag);
		fgrn::GenGrain _generator;
		std::vector <TaskInfo>
		               _task_list;
	};

	typedef std::shared_ptr <FrameProc> ProcSPtr;

	static void    redirect_task (avstp_TaskDispatcher *dispatcher_ptr, void *data_ptr);

	bool           _simd4_flag = false;
	bool           _avx_flag   = false;

	fgrn::VisionFilter
	               _filter;

	uint32_t       _seed_base  = 12345;
	bool           _cf_flag    = false; // Constant seed for all frames
	bool           _cp_flag    = false; // Constant seed for all planes of a frame
	bool           _draft_flag = false;

	AvstpWrapper & _avstp;

	// Mutex to lock before accessing the processor pool
	std::mutex     _mtx_pool;
	std::vector <ProcSPtr>
	               _proc_pool;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	               GrainProc ()                               = delete;
	               GrainProc (const GrainProc &other)         = delete;
	               GrainProc (GrainProc &&other)              = delete;
	GrainProc &    operator = (const GrainProc &other)        = delete;
	GrainProc &    operator = (GrainProc &&other)             = delete;
	bool           operator == (const GrainProc &other) const = delete;
	bool           operator != (const GrainProc &other) const = delete;

}; // class GrainProc



}  // namespace chkdr



//#include "chkdr/GrainProc.hpp"



#endif   // chkdr_GrainProc_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
