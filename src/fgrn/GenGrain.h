/*****************************************************************************

        GenGrain.h
        Author: Laurent de Soras, 2022

Main grain generator class. Adds film grain to a single picture.

System-level threading is handled by the caller.
The object can process only one picture at once.

Usage for the multi-threaded interface:

- mt_start (), returns the actual number of threads N
- In parallel: N times mt_proc_pass1 (i)
- Wait for all the threads to finish + fence
- mt_prepare_pass2 ()
- In parallel: N times mt_proc_pass2 (i)
- Wait for all the threads to finish + fence

Algorithm from:
Alasdair Newson, Julie Delon, Bruno Galerne,
A Stochastic Film Grain Model for Resolution-Independent Rendering,
Computer Graphics Forum, Wiley, 2017
https://hal.archives-ouvertes.fr/hal-01520260

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (fgrn_GenGrain_HEADER_INCLUDED)
#define fgrn_GenGrain_HEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "fgrn/Cell.h"
#include "fgrn/CellCache.h"
#include "fgrn/GrainDensity.h"
#include "fgrn/PointList.h"
#include "fstb/VecAlign.h"
#include "fstb/Vf32.h"
#include "fstb/Vu32.h"

#include <array>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include <cstddef>
#include <cstdint>



namespace fgrn
{



class VisionFilter;

class GenGrain
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	typedef GenGrain ThisType;

	explicit       GenGrain (bool simd4_flag, bool avx_flag);

	// Single thread interface
	void           process (float *dst_ptr, const float *src_ptr, int w, int h, ptrdiff_t src_stride, ptrdiff_t dst_stride, const VisionFilter &filter, uint32_t pic_seed);

	// Multi-thread interface
	int            mt_start (float *dst_ptr, const float *src_ptr, int w, int h, ptrdiff_t src_stride, ptrdiff_t dst_stride, const VisionFilter &filter, uint32_t pic_seed, int max_nbr_threads);
	void           mt_proc_pass1 (int idx);
	void           mt_prepare_pass2 ();
	void           mt_proc_pass2 (int idx);

	// Reserved for the cache manager
	void           build_cell (Cell &cell, int px, int py) const;



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	// Context for a single thread
	class Context
	{
	public:
		// Start (incl) and end (excl) rows to process
		int            _y_beg = 0;
		int            _y_end = 0;

		CellCache      _cell_cache;
	};

	typedef std::array <int, 2> C2di; // Integer 2D coordinates
	typedef std::set <C2di> PixSet;

	// All the vision filter points grouped by covered area.
	// All coordinates are in pixels and relative to the filter center.
	typedef std::map <PixSet, PointList> FilterMap;

	void           render_part_fpu (Context &ctx);
	void           render_part_simd4 (Context &ctx);
#if fstb_ARCHI == fstb_ARCHI_X86
	void           render_part_avx (Context &ctx);
#endif

	template <typename F>
	void           render_part (Context &ctx, F check_inter);
	template <typename F>
	float          render_pixel (Context &ctx, int px, int py, F check_inter);
	const Cell &   use_cell (Context &ctx, int px, int py);

	bool           _simd4_flag = false;
	bool           _avx_flag   = false;

	// Picture size in pixels
	int            _pic_w = 0;
	int            _pic_h = 0;

	const float *  _src_ptr    = nullptr;
	ptrdiff_t      _src_stride = 0; // In pixels!

	float *        _dst_ptr    = nullptr;
	ptrdiff_t      _dst_stride = 0; // In pixels!

	const VisionFilter *
	               _filter_ptr = nullptr;

	// Multiplier to convert the number of intersections into a [0 ; 1] pixel
	// value
	float          _out_scale  = 0;

	// Grain radius average value in pixels and standard deviation
	float          _g_rad_mu = 0.04f; // > 0
	float          _g_rad_s  = 0.25f; // >= 0

	GrainDensity   _density;
	GrainDensity::DataGrain
	               _density_info;

	int            _nbr_threads = 0;
	std::vector <Context>
	               _ctx_arr;

	void (ThisType::*                   // 0 = not set
	               _render_part_ptr) (Context &ctx) = nullptr;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	               GenGrain ()                               = delete;
	               GenGrain (const GenGrain &other)          = delete;
	               GenGrain (GenGrain &&other)               = delete;
	GenGrain &     operator = (const GenGrain &other)        = delete;
	GenGrain &     operator = (GenGrain &&other)             = delete;
	bool           operator == (const GenGrain &other) const = delete;
	bool           operator != (const GenGrain &other) const = delete;

}; // class GenGrain



}  // namespace fgrn



#include "fgrn/GenGrain.hpp"



#endif   // fgrn_GenGrain_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
