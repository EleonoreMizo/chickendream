/*****************************************************************************

        main.cpp
        Author: Laurent de Soras, 2021

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://www.wtfpl.net/ for more details.

*Tab=3***********************************************************************/



#if defined (_MSC_VER)
	#pragma warning (4 : 4786 4800)
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "fstb/def.h"
#include "fstb/fnc.h"
#include "fgrn/GenGrain.h"
#include "fgrn/UtilPrng.h"
#include "fgrn/VisionFilter.h"

#if defined (_MSC_VER)
#include <crtdbg.h>
#include <new.h>
#endif   // _MSC_VER

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <new>
#include <vector>

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>



/*\\\ CLASS DEFINITIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



class Histogram
{
public:
	void           reset (double v_min, double v_max, int sz, std::string name);
	template <typename F>
	void           insert (int nbr_val, F f);
	void           insert (double val);
	void           print () const;
private:
	double         _v_min = 0;
	double         _v_max = 1;
	int            _sz    = 80;
	std::vector <int>
	               _hist { _sz, 0 };
	std::string    _name;
};

void	Histogram::reset (double v_min, double v_max, int sz, std::string name)
{
	assert (v_min < v_max);
	assert (sz >= 2);

	_v_min = v_min;
	_v_max = v_max;
	_sz    = sz;
	_hist.clear ();
	_hist.resize (_sz, 0);
	_name  = name;
}

template <typename F>
void	Histogram::insert (int nbr_val, F f)
{
	typedef std::chrono::high_resolution_clock ClkType;
	ClkType        clk;
	const auto     t_beg = clk.now ();
	for (int k = 0; k < nbr_val; ++k)
	{
		insert (f (k));
	}
	const auto     t_end = clk.now ();
	const auto     dur_s =
		  double ((t_end - t_beg).count ())
		* double (ClkType::period::num)
		/ double (ClkType::period::den);
	printf ("%d elements inserted in %.1f us\n", nbr_val, dur_s * 1e6);
}

void	Histogram::insert (double val)
{
	val = fstb::limit (val, _v_min, _v_max);
	const auto     pos_f = (_sz - 1) * (val - _v_min) / (_v_max - _v_min);
	const auto     pos_i = fstb::round_int (pos_f);
	++ _hist [pos_i];
}

void	Histogram::print () const
{
	constexpr int  height = 10;
	const int      n_max  = std::max (
		*std::max_element (_hist.begin (), _hist.end ()),
		1
	);
	const int      r_ofs  = n_max >> 1;
	for (int y = height; y >= 0; --y)
	{
		for (int x = 0; x < _sz; ++x)
		{
			const auto     v     = _hist [x];
			const auto     bar_h = (height * 2 * v + r_ofs) / n_max;
			// For code page 850
			const auto     c     =
				  (bar_h >= y * 2 + 1) ? '\xDB' // Could be '#'
				: (v == 0 && y == 0  ) ? '_'
				: (bar_h >= y * 2    ) ? '\xDC' // Could be '_'
				:                        ' ';
			printf ("%c", c);
		}
		printf ("\n");
	}
	printf ("%s, range: [%f; %f]\n\n", _name.c_str (), _v_min, _v_max);
}



/*\\\ FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



#if defined (_MSC_VER)
static int __cdecl	MAIN_new_handler_cb (size_t dummy)
{
	fstb::unused (dummy);
	throw std::bad_alloc ();
}
#endif	// _MSC_VER



#if defined (_MSC_VER) && ! defined (NDEBUG)
static int	__cdecl	MAIN_debug_alloc_hook_cb (int alloc_type, void *user_data_ptr, size_t size, int block_type, long request_nbr, const unsigned char *filename_0, int line_nbr)
{
	fstb::unused (user_data_ptr, size, request_nbr, filename_0, line_nbr);

	if (block_type != _CRT_BLOCK)	// Ignore CRT blocks to prevent infinite recursion
	{
		switch (alloc_type)
		{
		case _HOOK_ALLOC:
		case _HOOK_REALLOC:
		case _HOOK_FREE:

			// Put some debug code here

			break;

		default:
			assert (false);	// Undefined allocation type
			break;
		}
	}

	return 1;
}
#endif



#if defined (_MSC_VER) && ! defined (NDEBUG)
static int	__cdecl	MAIN_debug_report_hook_cb (int report_type, char *user_msg_0, int *ret_val_ptr)
{
	fstb::unused (user_msg_0);

	*ret_val_ptr = 0;	// 1 to override the CRT default reporting mode

	switch (report_type)
	{
	case _CRT_WARN:
	case _CRT_ERROR:
	case _CRT_ASSERT:

// Put some debug code here

		break;
	}

	return *ret_val_ptr;
}
#endif



static void	MAIN_main_prog_init ()
{
#if defined (_MSC_VER)
	::_set_new_handler (::MAIN_new_handler_cb);
#endif   // _MSC_VER

#if defined (_MSC_VER) && ! defined (NDEBUG)
	{
		const int      mode =   (1 * _CRTDBG_MODE_DEBUG)
		                      | (1 * _CRTDBG_MODE_WNDW);
		::_CrtSetReportMode (_CRT_WARN, mode);
		::_CrtSetReportMode (_CRT_ERROR, mode);
		::_CrtSetReportMode (_CRT_ASSERT, mode);

		const int      old_flags = ::_CrtSetDbgFlag (_CRTDBG_REPORT_FLAG);
		::_CrtSetDbgFlag (  old_flags
		                  | (1 * _CRTDBG_LEAK_CHECK_DF)
		                  | (1 * _CRTDBG_CHECK_ALWAYS_DF));
		::_CrtSetBreakAlloc (-1);	// Specify here a memory bloc number
		::_CrtSetAllocHook (MAIN_debug_alloc_hook_cb);
		::_CrtSetReportHook (MAIN_debug_report_hook_cb);

		// Speed up I/O but breaks C stdio compatibility
//		std::cout.sync_with_stdio (false);
//		std::cin.sync_with_stdio (false);
//		std::cerr.sync_with_stdio (false);
//		std::clog.sync_with_stdio (false);
	}
#endif	// _MSC_VER, NDEBUG
}



static void	MAIN_main_prog_end ()
{
#if defined (_MSC_VER) && ! defined (NDEBUG)
	{
		const int      mode =   (1 * _CRTDBG_MODE_DEBUG)
		                      | (0 * _CRTDBG_MODE_WNDW);
		::_CrtSetReportMode (_CRT_WARN, mode);
		::_CrtSetReportMode (_CRT_ERROR, mode);
		::_CrtSetReportMode (_CRT_ASSERT, mode);

		::_CrtMemState mem_state;
		::_CrtMemCheckpoint (&mem_state);
		::_CrtMemDumpStatistics (&mem_state);
	}
#endif   // _MSC_VER, NDEBUG
}



std::array <double, 2>	quasirandom_gaussian_2d (int n)
{
	// 2D quasirandom sequence, uniform distribution
	// http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/
	constexpr auto g  = 1.32471795724474602596; // Solution of x^3 = x - 1
	constexpr auto a1 = 1.0 /  g;
	constexpr auto a2 = 1.0 / (g * g);

	const auto     v0 = fmod (0.5 + a1 * n, 1.0);
	const auto     v1 = fmod (0.5 + a2 * n, 1.0);

	// Box-Muller transform, basic form, to get standard distribution
	// https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform
	constexpr auto twopi = fstb::PI * 2;

	const auto     r  = sqrt (-2 * log (std::max (v0, 1e-300)));
	const auto     an = twopi * v1;
	const auto     c  = cos (an);
	const auto     s  = sin (an);

	return { r * c, r * s };
}



// https://www.desmos.com/calculator/hbdwn4kugl
void	build_gaussian_distrib_2d (int nbr_points)
{
	const auto     w  = 1024;
	const auto     h  = w;
	const auto     sd = 128; // Standard deviation, in grid units
	std::vector <uint8_t> pic (w * h, uint8_t (0));

	const auto     cx = w / 2;
	const auto     cy = h / 2;

	for (int p_cnt = 0; p_cnt < nbr_points; ++p_cnt)
	{
		const auto     c2d = quasirandom_gaussian_2d (p_cnt);
		const auto     xf  = c2d [0];
		const auto     yf  = c2d [1];

		const auto     x = fstb::limit (fstb::round_int (xf * sd + cx), 0, w - 1);
		const auto     y = fstb::limit (fstb::round_int (yf * sd + cy), 0, h - 1);

		const auto     idx = y * w + x;
		pic [idx] = uint8_t (-1);
	}

	auto           f_ptr = fopen ("gaussian_distrib_2d.data", "wb");
	fwrite (pic.data (), sizeof (*pic.data ()), pic.size (), f_ptr);
	fclose (f_ptr);
}



std::vector <float>	compose_pic (int w, int h)
{
	auto           m  = [] (double x) {
		return 1 - fstb::sq (1 - x);
	};
	auto           s  = [] (double x, double a) {
		return 0.5 * (cos (exp (a * fstb::ipowpc <8> (x)) - 1) + 1);
	};
	auto           l  = [=] (double x, double a) {
		return fstb::lerp (m (x), s (x, a), fstb::ipowpc <8> (x));
	};

	std::vector <uint8_t> pic_g (w * h, uint8_t (0));
	std::vector <float> pic_l (w * h, 0.f);
	for (int y = 0; y < h; ++y)
	{
		const auto     ry = double (y) / double (h);
		const auto     vy = l (ry, 3.0);
		auto           row_g_ptr = pic_g.data () + y * w;
		auto           row_l_ptr = pic_l.data () + y * w;
		for (int x = 0; x < w; ++x)
		{
			const auto     rx = double (x) / double (w);
			const auto     vx = l (rx, 4.0);
			const auto     v  = 2 * (vx - 0.5) * (vy - 0.5) + 0.5;
			const auto     vb = fstb::limit (v, 0.0, 1.0);

			// Gamma
			const auto     vi = uint8_t (fstb::round_int (vb * 255));
			row_g_ptr [x] = vi;

			// Pseudo-linear light
			const auto     vl = float (fstb::sq (vb));
			row_l_ptr [x] = vl;
		}
	}

	auto           f_ptr = fopen ("original.data", "wb");
	fwrite (pic_g.data (), sizeof (*pic_g.data ()), pic_g.size (), f_ptr);
	fclose (f_ptr);

	return pic_l;
}

void	save_linear_pic (int w, int h, const std::vector <float> &pic_l)
{
	const auto     len = w * h;
	assert (len == int (pic_l.size ()));

	std::vector <uint8_t> pic_g (w * h);
	for (int k = 0; k < len; ++k)
	{
		const auto     v  = pic_l [k];
		const auto     vb = sqrtf (fstb::limit (v, 0.f, 1.f));
		const auto     vi = uint8_t (fstb::round_int (vb * 255));
		pic_g [k] = vi;
	}

	auto           f_ptr = fopen ("modified.data", "wb");
	fwrite (pic_g.data (), sizeof (*pic_g.data ()), pic_g.size (), f_ptr);
	fclose (f_ptr);
}



int main (int argc, char *argv [])
{
	fstb::unused (argc, argv);

	int            ret_val = 0;

	MAIN_main_prog_init ();

	try
	{
#if 0
		build_gaussian_distrib_2d (1'000);
#endif

#if 1
		using UP = fgrn::UtilPrng;
		Histogram      hist;

		constexpr int  nbr_elt =
# if defined (NDEBUG)
			1'000'000
# else
			100'000
# endif
			;
		hist.reset (0, 1, 80, "uniform");
		hist.insert (nbr_elt, [] (int k) {
			return UP::gen_uniform (uint32_t (12345 + k));
		});
		hist.print ();

		hist.reset (0, 4, 80, "log-norm, mu_log=0, sigma=0.25");
		hist.insert (nbr_elt, [] (int k) {
			return UP::gen_log_norm (uint32_t (12345 + k * 2), 0, 0.25f);
		});
		hist.print ();

		hist.reset (0, 79, 80, "poisson, lambda=29");
		hist.insert (nbr_elt, [] (int k) {
			return UP::gen_poisson (
				uint32_t (12345 + k * 2), fgrn::UtilPrng::_poisson_algo_cutoff - 1
			);
		});
		hist.print ();

		hist.reset (0, 79, 80, "poisson, lambda=30");
		hist.insert (nbr_elt, [] (int k) {
			return UP::gen_poisson (
				uint32_t (12345 + k * 2), fgrn::UtilPrng::_poisson_algo_cutoff
			);
		});
		hist.print ();
#endif

#if 0

		const auto     c1203 = fstb::Vu32 (1, 2, 0, 3);
		for (int k = 0; k < 4096; ++k)
		{
			const auto     lambda    = float (k) + 0.12345f;
			const auto     rnd_state = uint32_t (k * 37 + 1);
			const auto     ref = fgrn::UtilPrng::gen_poisson (rnd_state, lambda);
			const auto     tst = fgrn::UtilPrng::gen_poisson (
				fstb::Vu32 (rnd_state) + c1203, fstb::Vf32 (lambda)
			);
			const auto     dif = tst.template extract <2> () - ref;
			if (std::abs (dif) > 0)
			{
				printf ("Error. lambda = %f\n", lambda);
			}
		}

#endif

#if 0
		const int      w     = 256;
		const auto     h     = w;
		auto           pic_s = compose_pic (w, h);
		auto           pic_d = pic_s;

		using GG = fgrn::GenGrain;
		GG             grain_gen;

		typedef std::chrono::high_resolution_clock ClkType;
		ClkType        clk;
		const auto     t_beg = clk.now ();

		const fgrn::VisionFilter   vf (0.35f, 1000, 0.025f, 0.0f);
		grain_gen.process (pic_d.data (), pic_s.data (), w, h, w, h, vf, 12345);

		const auto     t_end = clk.now ();
		const auto     dur_s =
			  double ((t_end - t_beg).count ())
			* double (ClkType::period::num)
			/ double (ClkType::period::den);
		printf ("Duration: %.3f s\n", dur_s);

		save_linear_pic (w, h, pic_d);
#endif


		/*** To do ***/




	}

	catch (std::exception &e)
	{
		std::cout << "*** main() : Exception (std::exception) : " << e.what () << std::endl;
		throw;
	}

	catch (...)
	{
		std::cout << "*** main() : Undefined exception" << std::endl;
		throw;
	}

	MAIN_main_prog_end ();

	return ret_val;
}



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
