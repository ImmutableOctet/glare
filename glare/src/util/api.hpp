#pragma once

#define GLARE_EXPORT __declspec(dllexport)
#define GLARE_IMPORT __declspec(dllimport)

#ifdef GLARE_API_EXPORT_SYMBOLS
	#define GLARE_API GLARE_EXPORT
#else
	#define GLARE_API GLARE_IMPORT
#endif