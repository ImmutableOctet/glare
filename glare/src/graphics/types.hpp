#pragma once

#include "fwd.hpp"

// TODO: Determine if this should still be included here.
#include "misc.hpp"

#include <cstdint>

namespace graphics
{
	using NativeContext = void*;
	using ContextHandle = std::uint32_t; // GLint;

	using AnimationID = std::uint16_t;
	using BoneID = int; // std::int32_t; // std::uint16_t;

	// TODO: Implement half-size indices for smaller meshes.
	using MeshIndex = std::uint32_t; // GLuint;
}