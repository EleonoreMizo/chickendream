/*****************************************************************************

        main.cpp
        Author: Laurent de Soras, 2012

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#include "chkdrvs/Grain.h"
#include "chkdrvs/version.h"
#include "fstb/def.h"
#include "vsutl/Redirect.h"
#include "VapourSynth4.h"

#include <algorithm>



VS_EXTERNAL_API (void) VapourSynthPluginInit2 (::VSPlugin *plugin_ptr, const ::VSPLUGINAPI *api_ptr)
{
	api_ptr->configPlugin (
		chkdrvs_PLUGIN_NAME,
		chkdrvs_NAMESPACE,
		"Film grain generator",
		VS_MAKE_VERSION (chkdrvs_VERSION, 0),
		VAPOURSYNTH_API_VERSION,
		0, // VSPluginConfigFlags
		plugin_ptr
	);

	api_ptr->registerFunction ("grain",
		"clip:vnode;"
		"sigma:float:opt;"
		"res:int:opt;"
		"rad:float:opt;"
		"dev:float:opt;"
		"seed:int:opt;"
		"cf:int:opt;"
		"cp:int:opt;"
		"cpuopt:int:opt;"
	,	"clip:vnode;"
	,	&vsutl::Redirect <chkdrvs::Grain>::create, nullptr, plugin_ptr
	);
}



