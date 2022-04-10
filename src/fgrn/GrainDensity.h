/*****************************************************************************

        GrainDensity.h
        Author: Laurent de Soras, 2022

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (fgrn_GrainDensity_HEADER_INCLUDED)
#define fgrn_GrainDensity_HEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "fstb/def.h"
#include "fstb/VecAlign.h"
#include "fstb/Vf32.h"
#include "fstb/Vu32.h"

#include <atomic>

#include <cstdint>



namespace fgrn
{



class GrainDensity
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	typedef GrainDensity ThisType;

	explicit       GrainDensity (bool simd4_flag);

	class DataGrain
	{
	public:
		const int32_t *
		               _q_ptr      = nullptr;
		const uint32_t *
		               _seed_ptr   = nullptr;
		ptrdiff_t      _stride     = 0; // In pixels
		int64_t        _load_total = 0; // Arbitrary unit
	};

	// Alignment in bytes
	static constexpr int _align = 32;

	void           reset (int w, int h, float grain_radius_avg, float grain_radius_stddev, uint32_t pic_rnd_seed);
	void           process_area (int y_beg, int y_end, const float *lum_ptr, ptrdiff_t stride) noexcept;
	int64_t        get_load_row (int y) const noexcept;
	DataGrain      get_result () const noexcept;



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	// Scale for the CPU load (float to int)
	static constexpr double _load_mul = double (1ULL << 16);

	class ResultLambda
	{
	public:
		int32_t        _q         = 0;
		uint32_t       _rnd_state = 0;
	};

	void           process_area_fpu (int y_beg, int y_end, const float *lum_ptr, ptrdiff_t stride) noexcept;
	void           process_area_simd4 (int y_beg, int y_end, const float *lum_ptr, ptrdiff_t stride) noexcept;

	static fstb_FORCEINLINE float
	               process_row_fpu (int32_t * fstb_RESTRICT q_ptr, uint32_t * fstb_RESTRICT seed_ptr, uint32_t pic_rnd_seed, const float * fstb_RESTRICT lum_ptr, int x, int y, float lambda_mul, float eps_val, int w) noexcept;
	static fstb_FORCEINLINE float
	               compute_q (int32_t * fstb_RESTRICT q_ptr, uint32_t * fstb_RESTRICT seed_ptr, uint32_t pic_rnd_seed, const float * fstb_RESTRICT lum_ptr, int x, int y, float lambda_mul, float eps_val) noexcept;
	static fstb_FORCEINLINE fstb::Vf32
	               compute_q (int32_t * fstb_RESTRICT q_ptr, uint32_t * fstb_RESTRICT seed_ptr, fstb::Vu32 pic_rnd_seed, const float * fstb_RESTRICT lum_ptr, fstb::Vu32 x, fstb::Vu32 y, fstb::Vf32 lambda_mul_log2cst, fstb::Vf32 eps_val) noexcept;

	fstb::VecAlign <int32_t, _align>
	               _q_arr;

	fstb::VecAlign <uint32_t, _align>
	               _seed_arr;

	// CPU load (arbitrary unit) per picture row
	std::vector <int64_t>
	               _load_row_arr;

	// Total CPU load, arbitrary unit
	std::atomic <int64_t>
	               _load_total { 0 };

	// Picture size in pixels
	int            _w = 0;
	int            _h = 0;

	uint32_t       _pic_rnd_seed = 0;

	// Multiplier for the lambda calculation
	float          _lambda_mul = 1;

	// Positive value (relative to 1) to avoid div/0 and too large grain amount
	// for the brightest pixel value.
	float          _eps_val = 4e-4f;

	// In pixels
	ptrdiff_t      _stride  = 0;

	void (ThisType::*                   // 0 = not set
	               _process_area_ptr) (int y_beg, int y_end, const float *lum_ptr, ptrdiff_t stride) = nullptr;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	bool           operator == (const GrainDensity &other) const = delete;
	bool           operator != (const GrainDensity &other) const = delete;

}; // class GrainDensity



}  // namespace fgrn



//#include "fgrn/GrainDensity.hpp"



#endif   // fgrn_GrainDensity_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
