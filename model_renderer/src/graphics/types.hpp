#pragma once

#include <types.hpp>
#include <math/math.hpp>

namespace graphics
{
	using NativeContext = void*;

	using ContextHandle = unsigned int; // GLint;

	// TODO: Implement half-size indices for smaller meshes.
	using MeshIndex = std::uint32_t; // GLuint;

	enum class ElementType
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

	enum class Primitive
	{
		Point,
		Line,
		LineLoop,
		LineStrip,
		Triangle,
		TriangleStrip,
		TriangleFan,
		Quad,
		QuadStrip,
		Polygon,

		Unknown = -1,
	};

	struct VertexAttribute
	{
		ElementType type;

		int num_elements; // unsigned int
		int offset = 0;
	};

	// NOTE: Support for texture flags is driver-dependent.
	enum TextureFlags
	{
		None               = (1 << 0),

		// TODO: Review the best way to handle depth and stencil data:
		DepthMap           = (1 << 1),
		DepthStencilMap    = (1 << 2),

		Default            = None,
	};

	enum class BufferType : std::uint32_t
	{
		Color = (1 << 0),
		Depth = (1 << 1),
	};

	inline std::uint32_t operator& (BufferType a, BufferType b) { return (std::uint32_t)((std::uint32_t)a & (std::uint32_t)b); }
	inline BufferType operator~ (BufferType a) { return (BufferType)~(std::uint32_t)a; }
	inline BufferType operator| (BufferType a, BufferType b) { return (BufferType)((std::uint32_t)a | (std::uint32_t)b); }
	inline BufferType operator^ (BufferType a, BufferType b) { return (BufferType)((std::uint32_t)a ^ (std::uint32_t)b); }
	inline BufferType& operator|= (BufferType& a, BufferType b) { return (BufferType&)((std::uint32_t&)a |= (std::uint32_t)b); }
	inline BufferType& operator&= (BufferType& a, BufferType b) { return (BufferType&)((std::uint32_t&)a &= (std::uint32_t)b); }
	inline BufferType& operator^= (BufferType& a, BufferType b) { return (BufferType&)((std::uint32_t&)a ^= (std::uint32_t)b); }
}