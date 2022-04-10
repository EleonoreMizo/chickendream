/*****************************************************************************

        Cell.hpp
        Author: Laurent de Soras, 2021

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://www.wtfpl.net/ for more details.

*Tab=3***********************************************************************/



#if ! defined (fgrn_Cell_CODEHEADER_INCLUDED)
#define fgrn_Cell_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "fstb/fnc.h"
#include "fstb/Vf32.h"

#if fstb_ARCHI == fstb_ARCHI_X86
# include <immintrin.h>
#endif

#include <cassert>



namespace fgrn
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



void	Cell::resize (int sz)
{
	assert (sz >= 0);

	_centers._x_arr.resize (sz);
	_centers._y_arr.resize (sz);
	_r2_arr.resize (sz);
}



// Checks the intersection of a filter point with the list of grains.
// This is the "intensive loop" of the algorithm.
bool	Cell::check_intersect_fpu (float tx, float ty) const noexcept
{
	const auto     nbr_grains = int (_r2_arr.size ());
	assert (_centers.get_size () == nbr_grains);

	return check_intersect_fpu (
		tx, ty, nbr_grains,
		_centers._x_arr.data (),
		_centers._y_arr.data (),
		_r2_arr.data ()
	);
}



bool	Cell::check_intersect_simd4 (float tx, float ty) const noexcept
{
	const auto     nbr_grains = int (_r2_arr.size ());
	assert (_centers.get_size () == nbr_grains);

	return check_intersect_simd4 (
		tx, ty, nbr_grains,
		_centers._x_arr.data (),
		_centers._y_arr.data (),
		_r2_arr.data ()
	);
}



#if fstb_ARCHI == fstb_ARCHI_X86



bool	Cell::check_intersect_avx (float tx, float ty) const noexcept
{
	const auto     nbr_grains = int (_r2_arr.size ());
	assert (_centers.get_size () == nbr_grains);

	return check_intersect_avx (
		tx, ty, nbr_grains,
		_centers._x_arr.data (),
		_centers._y_arr.data (),
		_r2_arr.data ()
	);
}



#endif



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



bool	Cell::check_intersect_fpu (float tx, float ty, int nbr_grains, const float * fstb_RESTRICT cx_ptr, const float * fstb_RESTRICT cy_ptr, const float * fstb_RESTRICT r2_ptr) noexcept
{
	for (int pos = 0; pos < nbr_grains; ++pos)
	{
		const auto     dx = tx - cx_ptr [pos];
		const auto     dy = ty - cy_ptr [pos];
		const auto     r2 = r2_ptr [pos];
		const auto     d2 = fstb::sq (dx) + fstb::sq (dy);
		if (d2 < r2)
		{
			return true;
		}
	}

	return false;
}



bool	Cell::check_intersect_simd4 (float tx, float ty, int nbr_grains, const float * fstb_RESTRICT cx_ptr, const float * fstb_RESTRICT cy_ptr, const float * fstb_RESTRICT r2_ptr) noexcept
{
	constexpr int  simd_w = fstb::Vf32::_length;
	const auto     nx     = nbr_grains & ~(simd_w - 1);

	const auto     txv = fstb::Vf32 (tx);
	const auto     tyv = fstb::Vf32 (ty);
	for (int pos = 0; pos < nx; pos += simd_w)
	{
		const auto     cxv = fstb::Vf32::load (cx_ptr + pos);
		const auto     cyv = fstb::Vf32::load (cy_ptr + pos);
		const auto     r2v = fstb::Vf32::load (r2_ptr + pos);
		const auto     dxv = txv - cxv;
		const auto     dyv = tyv - cyv;
		const auto     d2v = fstb::sq (dxv) + fstb::sq (dyv);
		const auto     hit = (d2v < r2v);
		if (hit.or_h ())
		{
			return true;
		}
	}

	return check_intersect_fpu (
		tx, ty, nbr_grains - nx, cx_ptr + nx, cy_ptr + nx, r2_ptr + nx
	);
}



#if fstb_ARCHI == fstb_ARCHI_X86



bool	Cell::check_intersect_avx (float tx, float ty, int nbr_grains, const float * fstb_RESTRICT cx_ptr, const float * fstb_RESTRICT cy_ptr, const float * fstb_RESTRICT r2_ptr) noexcept
{
	constexpr int  simd_w = 8;
	const auto     nx     = nbr_grains & ~(simd_w - 1);

	const auto     txv = _mm256_set1_ps (tx);
	const auto     tyv = _mm256_set1_ps (ty);
	for (int pos = 0; pos < nx; pos += simd_w)
	{
		const auto     cxv = _mm256_load_ps (cx_ptr + pos);
		const auto     cyv = _mm256_load_ps (cy_ptr + pos);
		const auto     r2v = _mm256_load_ps (r2_ptr + pos);
		const auto     dxv = _mm256_sub_ps (txv, cxv);
		const auto     dyv = _mm256_sub_ps (tyv, cyv);
		const auto     d2v = _mm256_add_ps (
			_mm256_mul_ps (dxv, dxv),
			_mm256_mul_ps (dyv, dyv)
		);
		const auto     hit = _mm256_cmp_ps (d2v, r2v, _CMP_LT_OQ);
		if (_mm256_movemask_ps (hit) != 0)
		{
			_mm256_zeroupper ();	// Back to SSE state
			return true;
		}
	}

	_mm256_zeroupper ();	// Back to SSE state

	return check_intersect_fpu (
		tx, ty, nbr_grains - nx, cx_ptr + nx, cy_ptr + nx, r2_ptr + nx
	);
}



#endif // fstb_ARCHI_X86



}  // namespace fgrn



#endif   // fgrn_Cell_CODEHEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
