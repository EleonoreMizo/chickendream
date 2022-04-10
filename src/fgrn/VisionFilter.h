/*****************************************************************************

        VisionFilter.h
        Author: Laurent de Soras, 2022

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (fgrn_VisionFilter_HEADER_INCLUDED)
#define fgrn_VisionFilter_HEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "fgrn/PointList.h"

#include <array>
#include <map>
#include <memory>
#include <set>

#include <cstdint>



namespace fgrn
{



class VisionFilter
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	typedef std::array <int, 2> C2di; // Integer 2D coordinates
	typedef std::set <C2di> PixSet;

	// All the vision filter points grouped by covered area.
	// All coordinates are in pixels and relative to the filter center.
	typedef std::map <PixSet, PointList> FilterMap;

	explicit       VisionFilter (float sigma, int nbr_points, float grain_radius_avg, float grain_radius_stddev);
	               VisionFilter (const VisionFilter &other)  = default;
	               VisionFilter (VisionFilter &&other)       = default;
	               ~VisionFilter ()                          = default;
	VisionFilter & operator = (const VisionFilter &other)    = default;
	VisionFilter & operator = (VisionFilter &&other)         = default;

	inline float   get_grain_radius_avg () const noexcept;
	inline float   get_grain_radius_stddev () const noexcept;
	inline int     get_w () const noexcept;
	inline int     get_h () const noexcept;
	inline int     get_nbr_points () const noexcept;
	inline const FilterMap &
	               use_map () const noexcept;



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	class ResultLambda
	{
	public:
		int            _q         = 0;
		uint32_t       _rnd_state = 0;
	};

	void           build_filter (float sigma, float grain_radius_avg, float grain_radius_stddev);
	void           compute_filter_area () noexcept;

	template <typename T>
	static void    add_and_wrap (T &acc, T inc) noexcept;

	// Number of points
	int            _nbr_points = 0;

	// Grain characteristics
	float          _rad_avg    = 0;
	float          _rad_stddev = 0;

	FilterMap      _filter;

	// Filter width and height in source pixels, > 0.
	// Helps for setting the size of the cell cache.
	int            _w = 0;
	int            _h = 0;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	               VisionFilter ()                               = delete;
	bool           operator == (const VisionFilter &other) const = delete;
	bool           operator != (const VisionFilter &other) const = delete;

}; // class VisionFilter



}  // namespace fgrn



#include "fgrn/VisionFilter.hpp"



#endif   // fgrn_VisionFilter_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
