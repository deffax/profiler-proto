#pragma once
#ifdef _WIN32
#ifndef VC_EXTRALEAN
#	define VC_EXTRALEAN			// Exclude rarely-used stuff from Windows headers
#endif

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINNT
#	define _WIN32_WINNT 0x0501	// We support Windows Server 2003, Windows XP or above
#endif
#	include <windows.h>
#endif