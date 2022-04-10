/*****************************************************************************

        AllocAlign.h
        Author: Laurent de Soras, 2011

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (fstb_AllocAlign_HEADER_INCLUDED)
#define	fstb_AllocAlign_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma warning (4 : 4250)
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include <memory>

#include <cstddef>
#include <cstdint>



namespace fstb
{



template <typename T, long ALIG = 16>
class AllocAlign
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	static constexpr long ALIGNMENT = ALIG;

	typedef	T	value_type;
	typedef	value_type *	pointer;
	typedef	const value_type *	const_pointer;
	typedef	value_type &	reference;
	typedef	const value_type &	const_reference;
	typedef	size_t	size_type;
	typedef	ptrdiff_t	difference_type;

	               AllocAlign ()                                   = default;
	               AllocAlign (AllocAlign <T, ALIG> const &other)  = default;
	template <typename U>
	               AllocAlign (AllocAlign <U, ALIG> const &/*other*/) {}
	               ~AllocAlign ()                                  = default;

	// Address
	[[deprecated]] inline pointer
	               address (reference r) noexcept;
	[[deprecated]] inline const_pointer
	               address (const_reference r) noexcept;

	// Memory allocation
	[[deprecated]] inline pointer
	               allocate (size_type n, const void *ptr);
	inline pointer allocate (size_type n);
	inline void    deallocate (pointer p, size_type n) noexcept;

	// Size
	[[deprecated]] inline size_type
	               max_size () const noexcept;

	// Construction/destruction
	[[deprecated]] inline void
	               construct (pointer ptr, const T & t);
	[[deprecated]] inline void
	               destroy (pointer ptr);

	inline bool    operator == (AllocAlign <T, ALIG> const &other) noexcept;
	inline bool    operator != (AllocAlign <T, ALIG> const &other) noexcept;

	template <typename U>
	struct rebind
	{
		typedef AllocAlign <U, ALIG> other;
	};



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

};	// class AllocAlign



}	// namespace fstb



#include "fstb/AllocAlign.hpp"



#endif	// fstb_AllocAlign_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
