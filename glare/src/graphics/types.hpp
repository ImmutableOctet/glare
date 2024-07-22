#pragma once

#include "fwd.hpp"

#include <cstdint>

namespace graphics
{
	using NativeContext = void*;
	using ContextHandle = std::uint32_t; // GLint;

	using AnimationIndex = std::uint16_t; // std::uint32_t;
	using BoneIndex      = std::uint16_t; // std::uint32_t;

	// TODO: Implement half-size indices for smaller meshes.
	using MeshIndex = std::uint32_t; // GLuint;
}