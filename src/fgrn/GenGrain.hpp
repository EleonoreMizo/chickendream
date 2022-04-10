/*****************************************************************************

        GenGrain.hpp
        Author: Laurent de Soras, 2021

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://www.wtfpl.net/ for more details.

*Tab=3***********************************************************************/



#if ! defined (fgrn_GenGrain_CODEHEADER_INCLUDED)
#define fgrn_GenGrain_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "fgrn/VisionFilter.h"

#include <cassert>



namespace fgrn
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



template <typename F>
void	GenGrain::render_part (Context &ctx, F check_inter)
{
	for (int y = ctx._y_beg; y < ctx._y_end; ++y)
	{
		for (int x = 0; x < _pic_w; ++x)
		{
			const auto     v = render_pixel (ctx, x, y, check_inter);
			_dst_ptr [y * _dst_stride + x] = v;
		}
	}
}



template <typename F>
float	GenGrain::render_pixel (Context &ctx, int px, int py, F check_inter)
{
	assert (px >= 0);
	assert (py < _pic_w);
	assert (px >= 0);
	assert (py < _pic_h);

	int            lum = 0;

	// Iterates on the groups
	const auto &   fmap = _filter_ptr->use_map ();
	for (auto &f_p : fmap)
	{
		const auto &   cell_list  = f_p.first;
		const auto &   point_list = f_p.second;

		// Test each point of the group
		const auto     nbr_points = point_list.get_size ();
		for (int p_cnt = 0; p_cnt < nbr_points; ++p_cnt)
		{
			const auto     fx = point_list._x_arr [p_cnt];
			const auto     fy = point_list._y_arr [p_cnt];

			// Check all the cells containing grains that could intersect with
			// the given filter point.
			for (const auto &cell_coord : cell_list)
			{
				// cx_r, cy_r are cell center coordinates relative to the filter
				// center.
				const auto     cx_r  = cell_coord [0];
				const auto     cy_r  = cell_coord [1];
				const auto     cx    = fstb::limit (px + cx_r, 0, _pic_w - 1);
				const auto     cy    = fstb::limit (py + cy_r, 0, _pic_h - 1);
				const auto &   cell  = use_cell (ctx, cx, cy);
				// tst_x and tst_y are the point coordinates relative to the
				// current cell center.
				const auto     tst_x = fx - float (cx_r);
				const auto     tst_y = fy - float (cy_r);
				if (check_inter (cell, tst_x, tst_y))
				{
					++ lum;
					break; // Escapes to the loop over filter points
				}
			}
		}
	}

	return float (lum) * _out_scale;
}



}  // namespace fgrn



#endif   // fgrn_GenGrain_CODEHEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
