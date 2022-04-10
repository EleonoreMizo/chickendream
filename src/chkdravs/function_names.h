/*****************************************************************************

        function_names.h
        Author: Laurent de Soras, 2021

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://www.wtfpl.net/ for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (chkdravs_function_names_HEADER_INCLUDED)
#define chkdravs_function_names_HEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



namespace chkdravs
{



#define chkdravs_NAMESPACE    "chkdr"

#define chkdravs_BUILD_NAME(x) chkdravs_NAMESPACE "_" x

#define chkdravs_GRAIN         chkdravs_BUILD_NAME ("grain")



}  // namespace chkdravs



//#include "chkdravs/function_names.hpp"



#endif   // chkdravs_function_names_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
