/*****************************************************************************

        GenGrain.cpp
        Author: Laurent de Soras, 2022

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/




/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "fgrn/GenGrain.h"
#include "fgrn/UtilPrng.h"
#include "fstb/fnc.h"
#include "fstb/Hash.h"
#include "fstb/Vs32.h"

#include <algorithm>

#include <cassert>



namespace fgrn
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



GenGrain::GenGrain (bool simd4_flag, bool avx_flag)
:	_simd4_flag (simd4_flag)
,	_avx_flag (avx_flag)
,	_density (simd4_flag)
,	_render_part_ptr (&ThisType::render_part_fpu)
{
	if (_simd4_flag)
	{
		_render_part_ptr = &ThisType::render_part_simd4;
	}
	if (_avx_flag)
	{
		_render_part_ptr = &ThisType::render_part_avx;
	}
}



void	GenGrain::process (float *dst_ptr, const float *src_ptr, int w, int h, ptrdiff_t src_stride, ptrdiff_t dst_stride, const VisionFilter &filter, uint32_t pic_seed, bool draft_flag)
{
	mt_start (
		dst_ptr, src_ptr,
		w, h,
		src_stride, dst_stride,
		filter, pic_seed, draft_flag, 1
	);
	mt_proc_pass1 (0);
	if (! draft_flag)
	{
		mt_prepare_pass2 ();
		mt_proc_pass2 (0);
	}
}



int	GenGrain::mt_start (float *dst_ptr, const float *src_ptr, int w, int h, ptrdiff_t src_stride, ptrdiff_t dst_stride, const VisionFilter &filter, uint32_t pic_seed, bool draft_flag, int max_nbr_threads)
{
	assert (w > 0);
	assert (h != 0);
	assert (src_ptr != nullptr);
	assert (dst_ptr != nullptr);
	assert (src_ptr != dst_ptr);

	_pic_w      = w;
	_pic_h      = h;
	_src_ptr    = src_ptr;
	_src_stride = src_stride;
	_dst_ptr    = dst_ptr;
	_dst_stride = dst_stride;
	_filter_ptr = &filter;

	const int      nbr_points = _filter_ptr->get_nbr_points ();
	_out_scale  = 1.f / float (nbr_points);

	_g_rad_mu   = filter.get_grain_radius_avg ();
	_g_rad_s    = filter.get_grain_radius_stddev ();

	_density.reset (w, h, _g_rad_mu, _g_rad_s, pic_seed, draft_flag);

	_nbr_threads = std::min (max_nbr_threads, h);
	_ctx_arr.resize (_nbr_threads);

	// Precomputes base data for each source pixel
	for (int t_cnt = 0; t_cnt < _nbr_threads; ++t_cnt)
	{
		auto &         ctx = _ctx_arr [t_cnt];
		ctx._y_beg = h *  t_cnt      / _nbr_threads;
		ctx._y_end = h * (t_cnt + 1) / _nbr_threads;
		assert (ctx._y_beg < ctx._y_end);
	}

	return _nbr_threads;
}



void	GenGrain::mt_proc_pass1 (int idx)
{
	assert (idx >= 0);
	assert (idx < _nbr_threads);

	auto &         ctx = _ctx_arr [idx];
	_density.process_area (
		ctx._y_beg, ctx._y_end,
		_src_ptr, _src_stride,
		_dst_ptr, _dst_stride
	);
}



void	GenGrain::mt_prepare_pass2 ()
{
	_density_info = _density.get_result ();

	// Evenly spreads the CPU load across threads
	const auto     load_tot = _density_info._load_total;
	int64_t        load_sum = 0;
	int            y        = 0;
	for (int t_cnt = 0; t_cnt < _nbr_threads; ++t_cnt)
	{
		assert (y < _pic_h);

		auto &         ctx = _ctx_arr [t_cnt];
		ctx._y_beg = y;

		const auto     load_target = load_tot * (t_cnt + 1) / _nbr_threads;
		do
		{
			const auto     load_row = _density.get_load_row (y);
			load_sum += load_row;
			++ y;
		}
		while (load_sum < load_target && y < _pic_h);

		ctx._y_end = y;
	}
	assert (y == _pic_h);
	assert (std::abs (load_sum - load_tot) < _nbr_threads); // Tolerance for rounding errors
}



void	GenGrain::mt_proc_pass2 (int idx)
{
	assert (idx >= 0);
	assert (idx < _nbr_threads);

	const int      filter_h = _filter_ptr->get_h ();
	auto &         ctx      = _ctx_arr [idx];
	ctx._cell_cache.reset (_pic_w, filter_h);

	(this->*_render_part_ptr) (ctx);
}



// Called by the cache manager on request
void	GenGrain::build_cell (Cell &cell, int px, int py) const
{
	assert (px >= 0);
	assert (py < _pic_w);
	assert (px >= 0);
	assert (py < _pic_h);

	const auto     d_index   = py * _density_info._stride + px;
	const auto     q         = _density_info._q_ptr [d_index];
	auto           rnd_state = _density_info._seed_ptr [d_index];
	cell.resize (q);

	constexpr int  simd_w = fstb::Vf32::_length;
	const auto     nx     = q & ~(simd_w - 1);

	// Generates the center coordinates

	const auto     vhalf = fstb::Vf32 (0.5f);
	const auto     vone  = fstb::Vu32 (1);
	const auto     vstep = fstb::Vs32 (simd_w * 2);
	auto           vrnd  = fstb::Vu32 (
		rnd_state, rnd_state + 2, rnd_state + 4, rnd_state + 6
	);
	for (int pos = 0; pos < nx; pos += simd_w)
	{
		const auto     cx = UtilPrng::gen_uniform (vrnd       ) - vhalf;
		const auto     cy = UtilPrng::gen_uniform (vrnd + vone) - vhalf;
		vrnd += vstep;
		cx.store (&cell._centers._x_arr [pos]);
		cy.store (&cell._centers._y_arr [pos]);
	}

	for (int pos = nx; pos < q; ++pos)
	{
		const auto     cx = UtilPrng::gen_uniform (rnd_state + pos * 2    ) - 0.5f;
		const auto     cy = UtilPrng::gen_uniform (rnd_state + pos * 2 + 1) - 0.5f;
		cell._centers._x_arr [pos] = cx;
		cell._centers._y_arr [pos] = cy;
	}

	// Constant radius
	if (_g_rad_s <= 0)
	{
		std::fill (
			cell._r2_arr.begin (), cell._r2_arr.end (), fstb::sq (_g_rad_mu)
		);
	}

	// Variable radius
	else
	{
		rnd_state = fstb::Hash::hash (rnd_state);

		const auto     mu_log = logf (_g_rad_mu);

		vrnd = fstb::Vu32 (
			rnd_state, rnd_state + 2, rnd_state + 4, rnd_state + 6
		);
		const auto     vmu_log = fstb::Vf32 (mu_log);
		const auto     vrad_s  = fstb::Vf32 (_g_rad_s);
		for (int pos = 0; pos < nx; pos += simd_w)
		{
			const auto     rad = UtilPrng::gen_log_norm (vrnd, vmu_log, vrad_s);
			vrnd += vstep;
			const auto     rad_sq = rad * rad;
			rad_sq.store (&cell._r2_arr [pos]);
		}

		for (int pos = nx; pos < q; ++pos)
		{
			const auto     rad =
				UtilPrng::gen_log_norm (rnd_state + pos * 2, mu_log, _g_rad_s);
			cell._r2_arr [pos] = fstb::sq (rad);
		}
	}
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



void	GenGrain::render_part_fpu (Context &ctx)
{
	render_part (ctx,
		[] (const Cell &cell, float px, float py)
		{
			return cell.check_intersect_fpu (px, py);
		}
	);
}



void	GenGrain::render_part_simd4 (Context &ctx)
{
	render_part (ctx,
		[] (const Cell &cell, float px, float py)
		{
			return cell.check_intersect_simd4 (px, py);
		}
	);
}



const Cell &	GenGrain::use_cell (Context &ctx, int cx, int cy)
{
	assert (cx >= 0);
	assert (cx < _pic_w);
	assert (cy >= 0);
	assert (cy < _pic_h);

	return ctx._cell_cache.use_cell (cx, cy, *this);
}



}  // namespace fgrn



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
