/*****************************************************************************

        Cell.h
        Author: Laurent de Soras, 2022

List of grains for a single source pixel
1 cell covers 1 source pixel

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (fgrn_Cell_HEADER_INCLUDED)
#define fgrn_Cell_HEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "fgrn/PointList.h"
#include "fstb/def.h"



namespace fgrn
{



class Cell
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	typedef PointList::VectF32Align VectF32Align;

	inline void    resize (int sz);
	fstb_FORCEINLINE bool
	               check_intersect_fpu (float tx, float ty) const noexcept;
	fstb_FORCEINLINE bool
	               check_intersect_simd4 (float tx, float ty) const noexcept;
#if fstb_ARCHI == fstb_ARCHI_X86
	fstb_FORCEINLINE bool
	               check_intersect_avx (float tx, float ty) const noexcept;
#endif

	// Grain coordinates in pixels, relative to the pixel origin (its center)
	PointList      _centers;

	// Corresponding squared grain radii, in pixels^2
	VectF32Align   _r2_arr;



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	fstb_FORCEINLINE static bool
	               check_intersect_fpu (float tx, float ty, int nbr_grains, const float * fstb_RESTRICT cx_ptr, const float * fstb_RESTRICT cy_ptr, const float * fstb_RESTRICT r2_ptr) noexcept;
	fstb_FORCEINLINE static bool
	               check_intersect_simd4 (float tx, float ty, int nbr_grains, const float * fstb_RESTRICT cx_ptr, const float * fstb_RESTRICT cy_ptr, const float * fstb_RESTRICT r2_ptr) noexcept;
#if fstb_ARCHI == fstb_ARCHI_X86
	fstb_FORCEINLINE static bool
	               check_intersect_avx (float tx, float ty, int nbr_grains, const float * fstb_RESTRICT cx_ptr, const float * fstb_RESTRICT cy_ptr, const float * fstb_RESTRICT r2_ptr) noexcept;
#endif



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	bool           operator == (const Cell &other) const = delete;
	bool           operator != (const Cell &other) const = delete;

}; // class Cell



}  // namespace fgrn



#include "fgrn/Cell.hpp"



#endif   // fgrn_Cell_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
