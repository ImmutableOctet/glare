#pragma once

#include <types.hpp>

#include "opengl/driver.hpp"

namespace graphics
{
	// An object representing the native objects
	// required to represent a complete mesh.
	union MeshComposition
	{
		GLMeshComposition gl;

		// Used for ABI compatibility.
		//std::uint8_t padding[64];
	};
}