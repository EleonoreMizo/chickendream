/*****************************************************************************

        UtilPrng.hpp
        Author: Laurent de Soras, 2021

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://www.wtfpl.net/ for more details.

*Tab=3***********************************************************************/



#if ! defined (fgrn_UtilPrng_CODEHEADER_INCLUDED)
#define fgrn_UtilPrng_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "fstb/Approx.h"
#include "fstb/def.h"
#include "fstb/fnc.h"
#include "fstb/Hash.h"
#include "fstb/ToolsSimd.h"
#include "fstb/Vs32.h"

#include <algorithm>

#include <cassert>



namespace fgrn
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



// Returns a random number in range [0 ; 1]
float	UtilPrng::gen_uniform (uint32_t rnd_state) noexcept
{
	constexpr auto mul = float (1.0 / double (UINT32_MAX));
	const auto     x_u = fstb::Hash::hash (rnd_state);
	const auto     val = float (x_u) * mul;

	return val;

/*** To do:
	Check if this cheaper formula is valid as an alternate hash for generating
	grain coordinates and radius (use at least the 17 lowest bits):
	a =  a ^ (a >>  4);
	a = (a ^ 0xDEADBEEF) + (a << 5);
	a =  a ^ (a >> 11);
	Source: Bob Jenkins, http://burtleburtle.net/bob/hash/integer.html

	Another thing to check:
	union Combo { float _f; uint32_t _i; };
	Combo val;
	val._i = (127 << 23) | (x_u >> 9);
	return val._f - 1.f;
***/
}



// Consumes two consecutive rnd_state
// lambda = average number of events
int	UtilPrng::gen_poisson (uint32_t rnd_state, float lambda) noexcept
{
	assert (lambda >= 0);

	const auto     u0 = gen_uniform (rnd_state);

	if (lambda >= _poisson_algo_cutoff)
	{
		// This formula uses a normal distribution as a rough approximation of
		// the Poisson PDF for large lambda.
		const auto     u1    = gen_uniform (rnd_state + 1);
		const auto     norm  = make_norm_from_uni (u0, u1).front ();
		const auto     avg   = lambda - 0.5f;
		const auto     equiv = std::max (norm * sqrtf (lambda) + avg, 0.f);
		const auto     n     = fstb::round_int (equiv);

		return n;
	}

	else
	{
		// https://en.wikipedia.org/wiki/Poisson_distribution#Generating_Poisson-distributed_random_variables
		// This formula (Inverse transform sampling) works only for small lambda
		// values because:
		// 1. exp (-lambda) may result in 0 if lambda is large enough.
		// 2. A large amount of iterations is not cheap.
		// 3. There is also a concern about the precision loss.
		// Using double precision would be of little help here, as lambda can be
		// several thousands.
		auto           prod  = expf (-lambda);
		int            n     = 0;
		auto           sum   = prod;
		// ceil() because we want n_max to be at least 1 for lambda < 1.
		const auto     n_max = fstb::ceil_int (lambda * 10);
		while (sum < u0 && n < n_max)
		{
			++ n;
			prod *= lambda / float (n);
			sum  += prod;
		}

		return n;
	}
}



std::array <float, 2>	UtilPrng::make_norm_from_uni (float u0, float u1) noexcept
{
	assert (u0 >= 0);
	assert (u0 <= 1);
	assert (u1 >= 0);
	assert (u1 <= 1);

	// Box-Muller transform, basic form, to get standard distribution
	// https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform
	constexpr auto twopi = float (fstb::PI * 2);

	const auto     r  = sqrtf (-2 * logf (std::max (u0, 1e-35f)));
	const auto     an = twopi * u1;
	const auto     c  = cosf (an);
	const auto     s  = sinf (an);

	return { r * c, r * s };
}



// Consumes two consecutive rnd_state
float	UtilPrng::gen_norm_trunc (uint32_t rnd_state) noexcept
{
	// Sums 6 10-bit random numbers, as an Irwin-Hall distribution
	// The output range is [-4.243 ; 4.243], keeping the variable extent
	// limited.
	// https://en.wikipedia.org/wiki/Irwin%E2%80%93Hall_distribution
	constexpr auto nbr  = 6;
	constexpr auto res  = 10;
	constexpr auto m    = (uint32_t (1) << res) - 1;
	constexpr auto avg  = int (m * nbr / 2);
	const auto     r0   = fstb::Hash::hash (rnd_state    );
	const auto     r1   = fstb::Hash::hash (rnd_state + 1);
	const auto     rsu  =
		  (r0 & m) + ((r0 >> res) & m) + ((r0 >> (2 * res)) & m)
		+ (r1 & m) + ((r1 >> res) & m) + ((r1 >> (2 * res)) & m);
	const auto     rss  = int (rsu) - avg;

	// Adjusts the standard deviation
	// scale = sqrt (12 / nbr)
	constexpr auto std_scale = float (fstb::SQRT2);
	constexpr auto mul  = std_scale / float (m);
	const auto     norm = float (rss) * mul;

	return norm;
}



// Consumes two consecutive rnd_state
float	UtilPrng::gen_log_norm (uint32_t rnd_state, float mu_log, float sigma) noexcept
{
	assert (mu_log >= -80); // Reasonable values for exp float
	assert (mu_log <= +80);
	assert (sigma >= 0);

	const auto     norm = gen_norm_trunc (rnd_state);
	const auto     val  = expf (mu_log + norm * sigma);

	return val;
}



fstb::Vf32	UtilPrng::gen_uniform (fstb::Vu32 x) noexcept
{
	// _mm_cvtepi32_ps works only on signed data, so we divide everything
	// by 2 to fit in the positive part. Resulting difference with the
	// scalar code is tiny, but does exist for some values.
	x = fstb::Hash::hash (x) >> 1;
	auto           f   = fstb::ToolsSimd::conv_s32_to_f32 (fstb::Vs32 (x));
	const auto     mul = fstb::Vf32 (float (1.0 / double (UINT32_MAX >> 1)));
	f *= mul;

	return f;
}



fstb::Vs32	UtilPrng::gen_poisson (fstb::Vu32 rnd_state, fstb::Vf32 lambda) noexcept
{
	// Use the standard distribution approximation by default, for all lambda
	// values
	const auto     u0    = gen_uniform (rnd_state                 );
	const auto     u1    = gen_uniform (rnd_state + fstb::Vu32 (1));
	const auto     norm  = make_norm_from_uni (u0, u1).front ();
	const auto     avg   = lambda - fstb::Vf32 (0.5f);
	const auto     equiv = max (norm * sqrt (lambda) + avg, fstb::Vf32 (0.f));
	auto           n     = fstb::ToolsSimd::round_f32_to_s32 (equiv);

	// If at least one lambda is small, we use the scalar algorithm for them
	const auto     small_v = (lambda < fstb::Vf32 (_poisson_algo_cutoff));
	if (small_v.or_h ())
	{
		const auto     mask     = small_v.movemask ();
		const auto     lambda_v = lambda.explode ();
		const auto     rnd      = rnd_state.explode ();
		std::tuple <int32_t, int32_t, int32_t, int32_t> n_small;
		gen_poisson_one <0> (n_small, lambda_v, rnd, mask);
		gen_poisson_one <1> (n_small, lambda_v, rnd, mask);
		gen_poisson_one <2> (n_small, lambda_v, rnd, mask);
		gen_poisson_one <3> (n_small, lambda_v, rnd, mask);
		n = select (fstb::ToolsSimd::cast_s32 (small_v), fstb::Vs32 (n_small), n);
	}

	return n;
}



std::array <fstb::Vf32, 2>	UtilPrng::make_norm_from_uni (fstb::Vf32 u0, fstb::Vf32 u1) noexcept
{
	assert ((u0 >= fstb::Vf32 (0)).and_h ());
	assert ((u0 <= fstb::Vf32 (1)).and_h ());
	assert ((u1 >= fstb::Vf32 (0)).and_h ());
	assert ((u1 <= fstb::Vf32 (1)).and_h ());

	const auto     m  = fstb::Vf32 (-2 * fstb::LN2);
	const auto     r  = sqrt (m * log2 (max (u0, fstb::Vf32 (1e-35f))));
	const auto     cs = cos_sin_twopi (u1);

	return { r * cs [0], r * cs [1] };
}



fstb::Vf32	UtilPrng::gen_norm_trunc (fstb::Vu32 rnd_state) noexcept
{
	constexpr auto nbr = 6;
	constexpr auto res = 10;
	constexpr auto m   = (uint32_t (1) << res) - 1;
	constexpr auto avg = int (nbr * m / 2);
	const auto     one = fstb::Vu32 (1);
	const auto     vm  = fstb::Vu32 (m);
	const auto     r0  = fstb::Hash::hash (rnd_state      );
	const auto     r1  = fstb::Hash::hash (rnd_state + one);
	const auto     a0  = fstb::Vs32 ( r0               & vm);
	const auto     a1  = fstb::Vs32 ((r0 >>  res     ) & vm);
	const auto     a2  = fstb::Vs32 ((r0 >> (res * 2)) & vm);
	const auto     a3  = fstb::Vs32 ( r1               & vm);
	const auto     a4  = fstb::Vs32 ((r1 >>  res     ) & vm);
	const auto     a5  = fstb::Vs32 ((r1 >> (res * 2)) & vm);
	const auto     a6  = fstb::Vs32 (avg);
	const auto     rss =
		  ((a0 + a1) +  a2      )
		+ ((a3 + a4) + (a5 - a6));

	// scale = sqrt (12 / nbr)
	constexpr auto std_scale = float (fstb::SQRT2);
	constexpr auto mul  = std_scale / float (m);
	const auto     vmu  = fstb::Vf32 (mul);
	const auto     norm = fstb::ToolsSimd::conv_s32_to_f32 (rss) * vmu;

	return norm;
}



fstb::Vf32	UtilPrng::gen_log_norm (fstb::Vu32 rnd_state, fstb::Vf32 mu_log, fstb::Vf32 sigma) noexcept
{
	const auto     norm = gen_norm_trunc (rnd_state);
	auto           earg = mu_log + norm * sigma;
	earg *= fstb::Vf32 (float (1 / fstb::LN2));
	const auto     val  = fstb::Approx::exp2 (earg);

	return val;
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



template <int N>
void	UtilPrng::gen_poisson_one (std::tuple <int32_t, int32_t, int32_t, int32_t> &n, const std::tuple <float, float, float, float> &x, const std::tuple <uint32_t, uint32_t, uint32_t, uint32_t> &rnd_state, unsigned int mask) noexcept
{
	if ((mask & (1 << N)) != 0)
	{
		std::get <N> (n) = gen_poisson (
			std::get <N> (rnd_state),
			std::get <N> (x)
		);
	}
}



// an in [0 ; 1]
// Returns { cos, sin } of an * 2 * pi
std::array <fstb::Vf32, 2>	UtilPrng::cos_sin_twopi (fstb::Vf32 an) noexcept
{
	assert ((an >= fstb::Vf32 (0)).and_h ());
	assert ((an <= fstb::Vf32 (1)).and_h ());

	const auto     one   = fstb::Vf32 ( 1.0);
	const auto     mone  = fstb::Vf32 (-1.0);
	const auto     two   = fstb::Vf32 ( 2.0);
	const auto     mtwo  = fstb::Vf32 (-2.0);
	const auto     three = fstb::Vf32 ( 3.0);
	const auto     four  = fstb::Vf32 ( 4.0);

	an *= four;                                   // [ 0 ; 1] -> [ 0 ; 4]

	auto           ans   = an;
	ans = select (ans > three,  ans - four, ans); // [ 0 ; 4] -> [-1 ; 3]
	ans = select (ans > one  ,  two - ans , ans); // [-1 ; 3] -> [-1 ; 1]
	const auto     s     = fstb::Approx::sin_rbj_halfpi (ans);

	auto           anc   = an - three;            // [ 0 ; 4] -> [-3 ; 1]
	anc = select (anc < mone , mtwo - anc , anc); // [-3 ; 1] -> [-1 ; 1]
	const auto     c     = fstb::Approx::sin_rbj_halfpi (anc);

	return { c, s };
}



}  // namespace fgrn



#endif   // fgrn_UtilPrng_CODEHEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
