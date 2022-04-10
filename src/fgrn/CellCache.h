/*****************************************************************************

        CellCache.h
        Author: Laurent de Soras, 2022

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (fgrn_CellCache_HEADER_INCLUDED)
#define fgrn_CellCache_HEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "fgrn/Cell.h"

#include <vector>



namespace fgrn
{



class GenGrain;

class CellCache
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

		void           reset (int w, int h);
		const Cell &   use_cell (int px, int py, GenGrain &cell_provider);


/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	class CellEntry
	{
	public:
		bool        _cached_flag = false;
		Cell        _cell;
	};

	inline void    move_h (int d) noexcept;
	inline void    move_v (int d) noexcept;
	inline void    invalidate_col (int c_x) noexcept;
	inline void    invalidate_row (int c_y) noexcept;

	// Cache size.
	// Minimum size: vision filter width * height
	// 0 = not initialised.
	int            _w        = 0;
	int            _h        = 0;

	// The cache is a circular buffer of lines, which are circular buffers
	// too.
	// Lines are stored contiguously (stride = _w).
	std::vector <CellEntry>
	               _storage;

	// Position of the cache top-left element in the picture
	int            _tl_x     = 0;
	int            _tl_y     = 0;

	// Position of the cache top-left element in the circular buffer
	int            _tl_x_pos = 0;
	int            _tl_y_pos = 0;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	bool           operator == (const CellCache &other) const = delete;
	bool           operator != (const CellCache &other) const = delete;

}; // class CellCache



}  // namespace fgrn



//#include "fgrn/CellCache.hpp"



#endif   // fgrn_CellCache_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
