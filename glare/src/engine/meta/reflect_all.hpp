#pragma once

namespace engine
{
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