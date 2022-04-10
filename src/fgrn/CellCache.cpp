/*****************************************************************************

        CellCache.cpp
        Author: Laurent de Soras, 2022

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/




/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "fgrn/CellCache.h"
#include "fgrn/GenGrain.h"

#include <algorithm>

#include <cassert>



namespace fgrn
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



void	CellCache::reset (int w, int h)
{
	assert (w > 0);
	assert (h > 0);

	_w        = w;
	_h        = h;
	_tl_x     = 0;
	_tl_y     = 0;
	_tl_x_pos = 0;
	_tl_y_pos = 0;
	_storage.resize (_w * _h);
	for (auto &ce : _storage)
	{
		ce._cached_flag = false;
	}
}



const Cell &	CellCache::use_cell (int px, int py, GenGrain &cell_provider)
{
	assert (_w > 0);
	assert (_h > 0);
	assert (px >= 0);
	assert (py >= 0);

	// Position out of the cache?
	if (   px < _tl_x || px >= _tl_x + _w
	    || py < _tl_y || py >= _tl_y + _h)
	{
		// No cell can be kept: we clear everything
		if (   px < _tl_x - (_w - 1) || px >= _tl_x + 2 * _w - 1
		    || py < _tl_y - (_h - 1) || py >= _tl_y + 2 * _h - 1)
		{
			for (auto &entry : _storage)
			{
				entry._cached_flag = false;
			}
			_tl_x     = px;
			_tl_y     = py;
			_tl_x_pos = 0;
			_tl_y_pos = 0;
		}

		// Moves the cache until we reach the requested cell
		else
		{
			while (px < _tl_x)
			{
				move_h (-1);
			}
			while (px >= _tl_x + _w)
			{
				move_h (+1);
			}
			while (py < _tl_y)
			{
				move_v (-1);
			}
			while (py >= _tl_y + _h)
			{
				move_v (+1);
			}
		}
	}
	assert (px >= _tl_x && px < _tl_x + _w && py >= _tl_y && py < _tl_y + _h);

	// Finds the cell within the cache
	auto           c_x = px - _tl_x + _tl_x_pos;
	if (c_x >= _w)
	{
		c_x -= _w;
	}
	auto           c_y = py - _tl_y + _tl_y_pos;
	if (c_y >= _h)
	{
		c_y -= _h;
	}
	const auto     c_pos = c_y * _w + c_x;
	auto &         entry = _storage [c_pos];
	if (! entry._cached_flag)
	{
		cell_provider.build_cell (entry._cell, px, py);
		entry._cached_flag = true;
	}

	return entry._cell;
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



void	CellCache::move_h (int d) noexcept
{
	invalidate_col ((_tl_x_pos + std::min (d, 0) + _w) % _w);
	_tl_x    += d;
	_tl_x_pos = (_tl_x_pos + d + _w) % _w;
}



void	CellCache::move_v (int d) noexcept
{
	invalidate_row ((_tl_y_pos + std::min (d, 0) + _h) % _h);
	_tl_y    += d;
	_tl_y_pos = (_tl_y_pos + d + _h) % _h;
}



void	CellCache::invalidate_col (int c_x) noexcept
{
	for (int k = 0; k < _h; ++k)
	{
		_storage [k * _w + c_x]._cached_flag = false;
	}
}



void	CellCache::invalidate_row (int c_y) noexcept
{
	for (int k = 0; k < _w; ++k)
	{
		_storage [c_y * _w + k]._cached_flag = false;
	}
}



}  // namespace fgrn



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
