#pragma once

#ifdef _WIN32
	#define GLARE_IMPORT __declspec(dllimport)
	#define GLARE_EXPORT __declspec(dllexport)
#else
	#define GLARE_IMPORT
	#define GLARE_EXPORT
#endif

#define GLARE_GAME_IMPORT // GLARE_IMPORT
#define GLARE_GAME_EXPORT // GLARE_EXPORT

#ifdef GLARE_GAME_EXPORT_SYMBOLS
	#define GLARE_GAME_API GLARE_GAME_EXPORT
#else
	#define GLARE_GAME_API GLARE_GAME_IMPORT
#endif // GLARE_GAME_EXPORT_SYMBOLS

#ifdef GLARE_ENGINE_EXPORT_SYMBOLS
	#define GLARE_ENGINE_API GLARE_EXPORT
	#define GLARE_ENGINE_SYMBOL GLARE_EXPORT
#else
	#define GLARE_ENGINE_API GLARE_IMPORT
	#define GLARE_ENGINE_SYMBOL GLARE_IMPORT
#endif // GLARE_ENGINE_EXPORT_SYMBOLS

#ifdef GLARE_UTIL_EXPORT_SYMBOLS
	#define GLARE_UTIL_API GLARE_EXPORT
#else
	#define GLARE_UTIL_API GLARE_IMPORT
#endif // GLARE_UTIL_EXPORT_SYMBOLS

#ifdef GLARE_SCRIPT_EXPORT_SYMBOLS
	#define GLARE_SCRIPT_API GLARE_EXPORT
#else
	#define GLARE_SCRIPT_API GLARE_IMPORT
#endif // GLARE_SCRIPT_EXPORT_SYMBOLS