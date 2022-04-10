/*****************************************************************************

        UtilPrng.h
        Author: Laurent de Soras, 2022

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (fgrn_UtilPrng_HEADER_INCLUDED)
#define fgrn_UtilPrng_HEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "fstb/Vf32.h"
#include "fstb/Vs32.h"
#include "fstb/Vu32.h"

#include <array>

#include <cstdint>



namespace fgrn
{



class UtilPrng
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	// More or less arbitrary limit to be refined, keeping in mind the balance
	// between accuracy and speed. At least, stay below 70.
	static constexpr float  _poisson_algo_cutoff = 30;

	static inline float
	               gen_uniform (uint32_t rnd_state) noexcept;
	static inline int
	               gen_poisson (uint32_t rnd_state, float lambda) noexcept;
	static inline std::array <float, 2>
	               make_norm_from_uni (float u0, float u1) noexcept;
	static inline float
	               gen_norm_trunc (uint32_t rnd_state) noexcept;
	static inline float
	               gen_log_norm (uint32_t rnd_state, float mu_log, float sigma) noexcept;

	static inline fstb::Vf32
	               gen_uniform (fstb::Vu32 x) noexcept;
	static inline fstb::Vs32
	               gen_poisson (fstb::Vu32 rnd_state, fstb::Vf32 lambda) noexcept;
	static inline std::array <fstb::Vf32, 2>
	               make_norm_from_uni (fstb::Vf32 u0, fstb::Vf32 u1) noexcept;
	static inline fstb::Vf32
	               gen_norm_trunc (fstb::Vu32 rnd_state) noexcept;
	static inline fstb::Vf32
	               gen_log_norm (fstb::Vu32 rnd_state, fstb::Vf32 mu_log, fstb::Vf32 sigma) noexcept;



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	template <int N>
	static inline void
	               gen_poisson_one (std::tuple <int32_t, int32_t, int32_t, int32_t> &n, const std::tuple <float, float, float, float> &x, const std::tuple <uint32_t, uint32_t, uint32_t, uint32_t> &rnd_state, unsigned int mask) noexcept;
	static inline std::array <fstb::Vf32, 2>
	               cos_sin_twopi (fstb::Vf32 an) noexcept;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	               UtilPrng ()                               = delete;
	               UtilPrng (const UtilPrng &other)          = delete;
	               UtilPrng (UtilPrng &&other)               = delete;
	UtilPrng &     operator = (const UtilPrng &other)        = delete;
	UtilPrng &     operator = (UtilPrng &&other)             = delete;
	bool           operator == (const UtilPrng &other) const = delete;
	bool           operator != (const UtilPrng &other) const = delete;

}; // class UtilPrng



}  // namespace fgrn



#include "fgrn/UtilPrng.hpp"



#endif   // fgrn_UtilPrng_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
