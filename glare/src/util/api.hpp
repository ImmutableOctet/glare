#pragma once

#ifdef _WIN32
	#define GLARE_IMPORT __declspec(dllimport)
	#define GLARE_EXPORT __declspec(dllexport)
#else
	#define GLARE_IMPORT
	#define GLARE_EXPORT
#endif

#ifdef GLARE_UTIL_EXPORT_SYMBOLS
	#define GLARE_UTIL_API GLARE_EXPORT
#else
	#define GLARE_UTIL_API GLARE_IMPORT
#endif

#ifdef GLARE_SCRIPT_EXPORT_SYMBOLS
	#define GLARE_SCRIPT_API GLARE_EXPORT
#else
	#define GLARE_SCRIPT_API GLARE_IMPORT
#endif