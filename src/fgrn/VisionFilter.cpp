/*****************************************************************************

        VisionFilter.cpp
        Author: Laurent de Soras, 2022

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/




/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "fgrn/VisionFilter.h"
#include "fgrn/UtilPrng.h"
#include "fstb/fnc.h"

#include <algorithm>

#include <cassert>



namespace fgrn
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



VisionFilter::VisionFilter (float sigma, int nbr_points, float grain_radius_avg, float grain_radius_stddev)
:	_nbr_points (nbr_points)
,	_rad_avg (grain_radius_avg)
,	_rad_stddev (grain_radius_stddev)
{
	assert (nbr_points > 0);
	assert (grain_radius_avg > 0);
	assert (grain_radius_stddev >= 0);

	build_filter (sigma, grain_radius_avg, grain_radius_stddev);
	compute_filter_area ();
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



void	VisionFilter::build_filter (float sigma, float grain_radius_avg, float grain_radius_stddev)
{
	_filter.clear ();

	// Approximation of an upper bound for the grain radius. This is a trade-
	// off between exhaustivity (accuracy) and performance.
	const auto     expected_grain_rad =
		grain_radius_avg * expf (grain_radius_stddev * 3);

	// Filter: limits the distance from the center to 4 times the standard
	// deviation. Again, an accuracy/performance trade-off.
	constexpr auto rmax = 4.f;

	// 2D quasirandom sequence, uniform distribution
	// http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/
	constexpr auto g   = 1.32471795724474602596; // Solution of x^(2+1) = x - 1
	constexpr auto a1  = 1.0 /  g;
	constexpr auto a2  = 1.0 / (g * g);

	double         a1n = 0.5;
	double         a2n = 0.5;

	for (int p_cnt = 0; p_cnt < _nbr_points; ++p_cnt)
	{
		std::array <float, 2> coord { float (a1n), float (a2n) };
		PixSet         cov;

		// Square-pixel filter: directly uses the uniform distribution, and
		// makes the value centered around 0.
		if (sigma <= 0)
		{
			for (auto &c : coord)
			{
				c -= 0.5f;
			}
			cov.insert ({ 0, 0 });
		}

		// Gaussian filter
		else
		{
			coord = UtilPrng::make_norm_from_uni (coord [0], coord [1]);

			auto           scale = sigma;
			const auto     r     =
				sqrtf (fstb::sq (coord [0]) + fstb::sq (coord [1]));
#if 0
			// Hard clipping
			scale *= rmax / std::max (r, rmax);
#else
			// Soft clipping
			// Not sure it makes a big difference with hard clipping
			scale *= powf (1.f + fstb::ipowpc <4> (r / rmax), -1.f / 4);
#endif

			for (auto &c : coord)
			{
				c *= scale;
			}

			// Finds the bounding box (source pixels) where the centers of the
			// potentially intersecting grains could be located.
			// These integer coordinates are inclusive
			const auto     x_min = fstb::round_int (coord [0] - expected_grain_rad);
			const auto     x_max = fstb::round_int (coord [0] + expected_grain_rad);
			const auto     y_min = fstb::round_int (coord [1] - expected_grain_rad);
			const auto     y_max = fstb::round_int (coord [1] + expected_grain_rad);

			// Use it directly
			for (int y = y_min; y <= y_max; ++y)
			{
				for (int x = x_min; x <= x_max; ++x)
				{
					cov.insert ({ x, y });
				}
			}
			/*** To do:
			We can still save some source pixels by checking the L2 norm
			and use something that could look more like a disc than a rectangle.
			This would be helpful for very large grains (huge zoom).
			***/
		}

		// Creates or reuse the group of source pixels
		auto &      point_list = _filter [cov];

		// Inserts the new point
		point_list._x_arr.push_back (coord [0]);
		point_list._y_arr.push_back (coord [1]);

		// Next
		add_and_wrap (a1n, a1);
		add_and_wrap (a2n, a2);
	}

	compute_filter_area ();
}



// Finds the bounding box for the whole filter
void	VisionFilter::compute_filter_area () noexcept
{
	assert (! _filter.empty ());

	int            min_x = 0;
	int            max_x = 0;
	int            min_y = 0;
	int            max_y = 0;
	for (auto &me : _filter)
	{
		const auto &   cell_set = me.first;
		for (const auto &coord : cell_set)
		{
			min_x = std::min (min_x, coord [0]);
			max_x = std::max (max_x, coord [0]);
			min_y = std::min (min_y, coord [1]);
			max_y = std::max (max_y, coord [1]);
		}
	}

	_w = max_x + 1 - min_x;
	_h = max_y + 1 - min_y;
}



template <typename T>
void	VisionFilter::add_and_wrap (T &acc, T inc) noexcept
{
	constexpr auto unit = T (1);
	acc += inc;
	if (acc >= unit)
	{
		acc -= unit;
	}
}



}  // namespace fgrn



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
