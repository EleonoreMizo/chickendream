/*****************************************************************************

        GrainDensity.cpp
        Author: Laurent de Soras, 2022

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/




/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "fstb/def.h"
#include "fgrn/GrainDensity.h"
#include "fgrn/UtilPrng.h"

#include <cassert>
#include <cmath>



namespace fgrn
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



constexpr int	GrainDensity::_align;



GrainDensity::GrainDensity (bool simd4_flag)
:	_process_area_ptr (&ThisType::process_area_fpu)
{
	if (simd4_flag)
	{
		_process_area_ptr = &ThisType::process_area_simd4;
	}
}



void	GrainDensity::reset (int w, int h, float grain_radius_avg, float grain_radius_stddev, uint32_t pic_rnd_seed)
{
	assert (w > 0);
	assert (h > 0);
	assert (grain_radius_avg > 0);
	assert (grain_radius_stddev >= 0);

	_w = w;
	_h = h;
	_pic_rnd_seed = pic_rnd_seed;
	constexpr int  align_pix = _align / sizeof (int32_t);
	_stride = w & ~(align_pix - 1);
	const auto     len = size_t (_stride * h);
	_q_arr.resize (len);
	_load_row_arr.resize (h);
	_seed_arr.resize (len);

	_load_total.store (0);

	// There is probably an error in the paper in the algorithm description
	// about the inclusion of grain_radius_stddev in the formula.
	// Expected value of a log-norm variable is exp (log_mu + 0.5 * sigma^2)
	// Expected value for its square is exp (2 * log_mu + 2 * sigma^2)
	// (found by integrating x^2 * PDF_lognorm(x) from 0 to +inf)
	_lambda_mul = float (-1.0 / (
		  fstb::PI
		* fstb::sq (grain_radius_avg)
		* expf (2 * fstb::sq (grain_radius_stddev))
	));
}



// lum_ptr points on the top-left of the whole picture. Values are expected
// to be in the [0 ; 1] range. Out-of-range values will just be clipped.
// stride is a pixel value.
// This function supports multiple simultaneous calls, but make sure the
// processed areas don't overlap.
void	GrainDensity::process_area (int y_beg, int y_end, const float *lum_ptr, ptrdiff_t stride) noexcept
{
	assert (_w > 0);
	assert (y_beg >= 0);
	assert (y_beg < y_end);
	assert (y_end <= _h);
	assert (lum_ptr != nullptr);

	(this->*_process_area_ptr) (y_beg, y_end, lum_ptr, stride);
}



int64_t	GrainDensity::get_load_row (int y) const noexcept
{
	assert (y >= 0);
	assert (y < _h);

	return _load_row_arr [y];
}



// Call this only when the whole picture has been processed.
GrainDensity::DataGrain	GrainDensity::get_result () const noexcept
{
	assert (_w > 0);

	return { _q_arr.data (), _seed_arr.data (), _stride, _load_total.load () };
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



constexpr double	GrainDensity::_load_mul;



void	GrainDensity::process_area_fpu (int y_beg, int y_end, const float *lum_ptr, ptrdiff_t stride) noexcept
{
	assert (_w > 0);
	assert (y_beg >= 0);
	assert (y_beg < y_end);
	assert (y_end <= _h);
	assert (lum_ptr != nullptr);

	int64_t        load_block = 0;

	lum_ptr += y_beg * stride;
	auto           q_ptr    = &_q_arr [y_beg * _stride];
	auto           seed_ptr = &_seed_arr [y_beg * _stride];
	for (int y = y_beg; y < y_end; ++y)
	{
		const auto     load_row = process_row_fpu (
			q_ptr, seed_ptr, _pic_rnd_seed, lum_ptr,
			0, y, _lambda_mul, _eps_val, _w
		);

		const auto     load_row_int = fstb::round_int64 (load_row * _load_mul);
		_load_row_arr [y] = load_row_int;

		q_ptr      += _stride;
		seed_ptr   += _stride;
		lum_ptr    += stride;
		load_block += load_row_int;
	}

	_load_total.fetch_add (load_block);
}



void	GrainDensity::process_area_simd4 (int y_beg, int y_end, const float *lum_ptr, ptrdiff_t stride) noexcept
{
	assert (_w > 0);
	assert (y_beg >= 0);
	assert (y_beg < y_end);
	assert (y_end <= _h);
	assert (lum_ptr != nullptr);

	int64_t        load_block = 0;

	constexpr int  simd_w = fstb::Vf32::_length;
	const auto     nx     = _w & ~(simd_w - 1);
	const auto     lml2_v = fstb::Vf32 (_lambda_mul * fstb::LN2);
	const auto     eps_v  = fstb::Vf32 (_eps_val);
	const auto     c0123  = fstb::Vu32 (0, 1, 2, 3);
	const auto     seed_v = fstb::Vu32 (_pic_rnd_seed);

	lum_ptr += y_beg * stride;
	auto           q_ptr    = &_q_arr [y_beg * _stride];
	auto           seed_ptr = &_seed_arr [y_beg * _stride];
	for (int y = y_beg; y < y_end; ++y)
	{
		const auto     y_v        = fstb::Vu32 (y);
		auto           load_row_v = fstb::Vf32::zero ();
		for (int x = 0; x < nx; x += simd_w)
		{
			const auto     x_v      = fstb::Vu32 (x) + c0123;
			const auto     load_pix = compute_q (
				q_ptr + x, seed_ptr + x, seed_v, lum_ptr + x,
				x_v, y_v, lml2_v, eps_v
			);
			load_row_v += load_pix;
		}

		auto           load_row = load_row_v.sum_h ();
		load_row += process_row_fpu (
			q_ptr, seed_ptr, _pic_rnd_seed, lum_ptr,
			nx, y, _lambda_mul, _eps_val, _w
		);

		const auto     load_row_int = fstb::round_int64 (load_row * _load_mul);
		_load_row_arr [y] = load_row_int;

		q_ptr      += _stride;
		seed_ptr   += _stride;
		lum_ptr    += stride;
		load_block += load_row_int;
	}

	_load_total.fetch_add (load_block);
}



// Pointers at the real beginning of the row (x = 0)
float	GrainDensity::process_row_fpu (int32_t * fstb_RESTRICT q_ptr, uint32_t * fstb_RESTRICT seed_ptr, uint32_t pic_rnd_seed, const float * fstb_RESTRICT lum_ptr, int x, int y, float lambda_mul, float eps_val, int w) noexcept
{
	assert (x <= w);

	float          load_row = 0;
	for ( ; x < w; ++x)
	{
		const auto     load_pix = compute_q (
			q_ptr + x, seed_ptr + x, pic_rnd_seed, lum_ptr + x,
			x, y, lambda_mul, eps_val
		);
		load_row += load_pix;
	}

	return load_row;
}



// Pointers are on the exact pixel location
float	GrainDensity::compute_q (int32_t * fstb_RESTRICT q_ptr, uint32_t * fstb_RESTRICT seed_ptr, uint32_t pic_rnd_seed, const float * fstb_RESTRICT lum_ptr, int x, int y, float lambda_mul, float eps_val) noexcept
{
	assert (x >= 0);
	assert (y >= 0);

	auto           rnd_state = pic_rnd_seed;
	rnd_state += uint32_t (y) << 20;
	rnd_state += uint32_t (x) <<  8;

	// We need another hash, because sequences may be long, and keeping the
	// original state could lead to similar sequence parts for neighbour pixels,
	// giving a feel of directional blur at high luminance.
	rnd_state  = fstb::Hash::hash (rnd_state);

	const auto     luminance = *lum_ptr;
	const auto     lum_neg   = fstb::limit (1.f - luminance, eps_val, 1.f);
	const auto     lambda    = lambda_mul * logf (lum_neg);
	const auto     q         = UtilPrng::gen_poisson (rnd_state, lambda);
	rnd_state += 2;

	*q_ptr     = q;
	*seed_ptr  = rnd_state;

	const auto     load = 1.09f - lum_neg;

	return load;
}



fstb::Vf32	GrainDensity::compute_q (int32_t * fstb_RESTRICT q_ptr, uint32_t * fstb_RESTRICT seed_ptr, fstb::Vu32 pic_rnd_seed, const float * fstb_RESTRICT lum_ptr, fstb::Vu32 x, fstb::Vu32 y, fstb::Vf32 lambda_mul_log2cst, fstb::Vf32 eps_val) noexcept
{
	auto           rnd_state = pic_rnd_seed;
	rnd_state += y << 20;
	rnd_state += x <<  8;
	rnd_state  = fstb::Hash::hash (rnd_state);

	const auto     luminance = fstb::Vf32::loadu (lum_ptr);
	const auto     one       = fstb::Vf32 (1.f);
	const auto     lum_neg   = min (max (one - luminance, eps_val), one);
	// The max() is not mandatory because it is not important in gen_poisson()
	// if lambda is slightly negative (approx could cause log2 to be < 0).
	// But it's better to play it safe and avoid assert() in debug.
	const auto     zero      = fstb::Vf32::zero ();
	const auto     lambda    = max (lambda_mul_log2cst * log2 (lum_neg), zero);
	const auto     q         = UtilPrng::gen_poisson (rnd_state, lambda);
	rnd_state += fstb::Vu32 (2);

	q.store (q_ptr);
	rnd_state.store (seed_ptr);

	const auto     load = fstb::Vf32 (1.09f) - lum_neg;

	return load;
}



}  // namespace fgrn



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
