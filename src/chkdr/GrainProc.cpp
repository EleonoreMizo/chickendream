/*****************************************************************************

        GrainProc.cpp
        Author: Laurent de Soras, 2022

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/




/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "chkdr/GrainProc.h"
#include "fstb/def.h"
#include "AvstpWrapper.h"

#if fstb_ARCHI == fstb_ARCHI_X86
# include <mmintrin.h>
#endif

#include <cassert>



namespace chkdr
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



GrainProc::GrainProc (float sigma, int res, float rad, float dev, uint32_t seed, bool cf_flag, bool cp_flag, bool draft_flag, bool simd4_flag, bool avx_flag)
:	_simd4_flag (simd4_flag)
,	_avx_flag (avx_flag)
,	_filter (sigma, res, rad, dev)
,	_seed_base (seed)
,	_cf_flag (cf_flag)
,	_cp_flag (cp_flag)
,	_draft_flag (draft_flag)
,	_avstp (AvstpWrapper::use_instance ())
{
	assert (check_sigma (sigma));
	assert (check_res (res));
	assert (check_rad (rad));
	assert (check_dev (dev));
}



// Strides in bytes
void	GrainProc::process_plane (uint8_t *dst_ptr, ptrdiff_t dst_stride, const uint8_t *src_ptr, ptrdiff_t src_stride, int w, int h, int frame_idx, int plane_idx)
{
	assert (dst_ptr != nullptr);
	assert (src_ptr != nullptr);
	assert (w > 0);
	assert (h > 0);
	assert (frame_idx >= 0);
	assert (plane_idx >= 0);

	const auto     seed = uint32_t (
			_seed_base
		+ ((_cp_flag) ? 0 : plane_idx    )
		+ ((_cf_flag) ? 0 : frame_idx * 4)
	);

	// Recycles a generator from the pool or creates a new one if empty
	ProcSPtr       proc_sptr;
	{
		std::lock_guard <std::mutex> lock (_mtx_pool);
		if (_proc_pool.empty ())
		{
			proc_sptr = std::make_shared <FrameProc> (_simd4_flag, _avx_flag);
		}
		else
		{
			proc_sptr = _proc_pool.back ();
			_proc_pool.pop_back ();
		}
	}

#if 0 // Single thread

	proc_sptr->_generator.process (
		reinterpret_cast <      float *> (dst_ptr),
		reinterpret_cast <const float *> (src_ptr),
		w, h,
		dst_stride / sizeof (float),
		src_stride / sizeof (float),
		_filter, seed
	);

#elif 0 // Multi-thread, standard

	const int      max_nbr_threads = 32;

	// Pass 1
	const int      nbr_threads = proc_sptr->_generator.mt_start (
		reinterpret_cast <      float *> (dst_ptr),
		reinterpret_cast <const float *> (src_ptr),
		w, h,
		dst_stride / sizeof (float),
		src_stride / sizeof (float),
		_filter, seed, max_nbr_threads
	);

	std::vector <std::thread> thread_arr (nbr_threads);

	for (int t_cnt = 0; t_cnt < nbr_threads; ++t_cnt)
	{
		thread_arr [t_cnt] = std::thread ([t_cnt, &proc_sptr] () {
			proc_sptr->_generator.mt_proc_pass1 (t_cnt);
		});
	}
	for (auto &thread : thread_arr)
	{
		thread.join ();
	}
	std::atomic_thread_fence (std::memory_order_seq_cst);

	// Pass 2
	if (! _draft_flag)
	{
		proc_sptr->_generator.mt_prepare_pass2 ();

		for (int t_cnt = 0; t_cnt < nbr_threads; ++t_cnt)
		{
			thread_arr [t_cnt] = std::thread ([t_cnt, &proc_sptr] () {
				proc_sptr->_generator.mt_proc_pass2 (t_cnt);
			});
		}
		for (auto &thread : thread_arr)
		{
			thread.join ();
		}
		std::atomic_thread_fence (std::memory_order_seq_cst);
	}

#else // Multi-thread, AVSTP

	const auto     max_nbr_threads = _avstp.get_nbr_threads ();
	AvstpScopedDispatcher   dispatcher (_avstp);

	// Pass 1
	const int      nbr_threads = proc_sptr->_generator.mt_start (
		reinterpret_cast <      float *> (dst_ptr),
		reinterpret_cast <const float *> (src_ptr),
		w, h,
		dst_stride / sizeof (float),
		src_stride / sizeof (float),
		_filter, seed, _draft_flag, max_nbr_threads
	);
	proc_sptr->_task_list.resize (nbr_threads);

	for (int t_cnt = 0; t_cnt < nbr_threads; ++t_cnt)
	{
		auto &      task = proc_sptr->_task_list [t_cnt];
		task._gen_ptr = &proc_sptr->_generator;
		task._pass    = 1;
		task._tid     = t_cnt;
		_avstp.enqueue_task (dispatcher._ptr, &redirect_task, &task);
	}
	_avstp.wait_completion (dispatcher._ptr);

	// Pass 2
	if (! _draft_flag)
	{
		proc_sptr->_generator.mt_prepare_pass2 ();

		for (int t_cnt = 0; t_cnt < nbr_threads; ++t_cnt)
		{
			auto &      task = proc_sptr->_task_list [t_cnt];
			task._pass = 2;
			_avstp.enqueue_task (dispatcher._ptr, &redirect_task, &task);
		}
		_avstp.wait_completion (dispatcher._ptr);
	}

#endif // Threading variants

	// Puts back the generator into the pool
	{
		std::lock_guard <std::mutex> lock (_mtx_pool);
		_proc_pool.push_back (proc_sptr);
	}
}



bool	GrainProc::check_sigma (float sigma) noexcept
{
	return (sigma >= 0 && sigma <= 1);
}



bool	GrainProc::check_res (int res) noexcept
{
	return (res > 0);
}



bool	GrainProc::check_rad (float rad) noexcept
{
	return (rad > 0);
}



bool	GrainProc::check_dev (float dev) noexcept
{
	return (dev >= 0 && dev <= 1);
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



GrainProc::FrameProc::FrameProc (bool simd4_flag, bool avx_flag)
:	_generator (simd4_flag, avx_flag)
{
	// Nothing
}



void	GrainProc::redirect_task (avstp_TaskDispatcher *dispatcher_ptr, void *data_ptr)
{
	fstb::unused (dispatcher_ptr);

#if fstb_ARCHI == fstb_ARCHI_X86
# if ! defined (_WIN64) && ! defined (__64BIT__) && ! defined (__amd64__) && ! defined (__x86_64__)
	// We can arrive here from any thread and we don't know the state of the
	// FP/MMX registers (it could have been changed by the previous filter that
	// used avstp) so we explicitely switch to the FP registers as we're going
	// to use them.
	_mm_empty ();
# endif
#endif

	TaskInfo &     task = *reinterpret_cast <TaskInfo *> (data_ptr);
	switch (task._pass)
	{
	case 1:
		task._gen_ptr->mt_proc_pass1 (task._tid);
		break;
	case 2:
		task._gen_ptr->mt_proc_pass2 (task._tid);
		break;
	default:
		assert (false);
		break;
	}
}



}  // namespace chkdr



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
