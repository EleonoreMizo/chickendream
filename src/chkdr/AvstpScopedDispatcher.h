/*****************************************************************************

        AvstpScopedDispatcher.h
        Author: Laurent de Soras, 2022

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (chkdr_AvstpScopedDispatcher_HEADER_INCLUDED)
#define chkdr_AvstpScopedDispatcher_HEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "avstp.h"



class AvstpWrapper;

namespace chkdr
{



class AvstpScopedDispatcher
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	explicit       AvstpScopedDispatcher (AvstpWrapper &avstp);
	               ~AvstpScopedDispatcher ();

	AvstpWrapper & _avstp;
	avstp_TaskDispatcher * const
	               _ptr;



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	               AvstpScopedDispatcher ()                               = delete;
	               AvstpScopedDispatcher (const AvstpScopedDispatcher &other) = delete;
	               AvstpScopedDispatcher (AvstpScopedDispatcher &&other)  = delete;
	AvstpScopedDispatcher &
	               operator = (const AvstpScopedDispatcher &other)        = delete;
	AvstpScopedDispatcher &
	               operator = (AvstpScopedDispatcher &&other)             = delete;
	bool           operator == (const AvstpScopedDispatcher &other) const = delete;
	bool           operator != (const AvstpScopedDispatcher &other) const = delete;

}; // class AvstpScopedDispatcher



}  // namespace chkdr



//#include "chkdr/AvstpScopedDispatcher.hpp"



#endif   // chkdr_AvstpScopedDispatcher_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
