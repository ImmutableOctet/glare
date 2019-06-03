#pragma once

#include "opengl/driver.hpp"

/*
	TODO:
		* Change 'MeshComposition' from a union to a variant.
*/

namespace graphics
{
	// API:

	// An object representing the native objects
	// required to represent a complete mesh.
	union MeshComposition
	{
		GLMeshComposition gl;

		// Used for ABI compatibility.
		//std::uint8_t padding[64];
	};
}