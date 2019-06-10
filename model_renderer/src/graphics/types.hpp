#pragma once

#include <types.hpp>
#include <math/math.hpp>

namespace graphics
{
	using ContextHandle = unsigned int; // GLint;

	// TODO: Implement half-size indices for smaller meshes.
	using MeshIndex = std::uint32_t; // GLuint;
}