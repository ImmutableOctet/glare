#pragma once

#if GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY
#include <util/api.hpp>
#endif // GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY

namespace engine
{
	namespace impl
	{
#if GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY
		struct reflect_all_t
		{
			reflect_all_t();
		};
#endif // GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY
	}

	/*
		Generates reflection meta-data for the `engine` module.
		Reflected meta-data can be queried via EnTT's meta-type system.
		
		NOTES:
			* Although this is declared here, the function definition can be found in the `reflection` submodule.
			(This avoids multiple template instantiations)

			* Calling the `engine::reflect` free-function with `T=void` or with no
			template-specification will also result in a call to this function.

			* Calling this function multiple times on the same thread is safe.
			
			* Calling this function from more than one thread concurrently is considered unsafe.
			(TODO: Look into fixing this)

			* The `Game` type automatically calls this function during construction.
			
			* The implementation of this function dictates the order-of-operations for reflected types.
			It is recommended that you do not inspect reflected types whilst meta factories are still being constructed.

			* The `primitives` argument is used to control whether we reflect simple enumeration types, structs, etc.
			* The `dependencies` argument determines if other supporting modules (e.g. `math`) are reflected.
	*/
	void reflect_all(bool primitives=true, bool dependencies=true);
}

#if GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY

extern "C" const struct GLARE_ENGINE_SYMBOL engine::impl::reflect_all_t glare_impl_engine_execute_reflect_all;

// NOTE: 32-bit extern-C symbols have slightly different symbol names on MSVC. (leading '_')
#ifdef GLARE_EXPORT_SYMBOLS
    #if defined(_WIN64)
        __pragma(comment (linker, "/export:glare_impl_engine_execute_reflect_all"))
    #else if defined(_WIN32)
        __pragma(comment (linker, "/export:_glare_impl_engine_execute_reflect_all"))
    #endif
#else
    #if defined(_WIN64)
        __pragma(comment (linker, "/include:glare_impl_engine_execute_reflect_all"))
    #else if defined(_WIN32)
        __pragma(comment (linker, "/include:_glare_impl_engine_execute_reflect_all"))
    #endif
#endif

#endif // GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY