/*****************************************************************************

        VisionFilter.hpp
        Author: Laurent de Soras, 2021

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://www.wtfpl.net/ for more details.

*Tab=3***********************************************************************/



#if ! defined (fgrn_VisionFilter_CODEHEADER_INCLUDED)
#define fgrn_VisionFilter_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



namespace fgrn
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



float	VisionFilter::get_grain_radius_avg () const noexcept
{
   return _rad_avg;
}



float	VisionFilter::get_grain_radius_stddev () const noexcept
{
   return _rad_stddev;
}



int	VisionFilter::get_w () const noexcept
{
   return _w;
}



int	VisionFilter::get_h () const noexcept
{
   return _h;
}



int VisionFilter::get_nbr_points () const noexcept
{
   return _nbr_points;
}



const VisionFilter::FilterMap &	VisionFilter::use_map () const noexcept
{
   return _filter;
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



}  // namespace fgrn



#endif   // fgrn_VisionFilter_CODEHEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
