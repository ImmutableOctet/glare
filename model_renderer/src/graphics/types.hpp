#pragma once

#include <types.hpp>
#include <math/math.hpp>

namespace graphics
{
	using NativeContext = void*;

	using ContextHandle = unsigned int; // GLint;

	// TODO: Implement half-size indices for smaller meshes.
	using MeshIndex = std::uint32_t; // GLuint;

	enum VertexElementType
	{
		Byte,
		UByte,
		Short,
		UShort,
		Int,
		UInt,
		Half,
		Float,
		Double,

		Char = Byte, // UByte
		Unknown = -1,
	};

	struct VertexAttribute
	{
		VertexElementType type;

		int num_elements; // unsigned int
		int offset = 0;
	};

	// NOTE: Support for texture flags is driver-dependent.
	enum TextureFlags
	{
		None = 0,

		Default = None,
	};
}