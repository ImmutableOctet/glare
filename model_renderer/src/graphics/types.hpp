#pragma once

#include <types.hpp>
#include <math/math.hpp>

#include <utility>

#include <string>
#include <variant>

namespace graphics
{
	using NativeContext = void*;

	using ContextHandle = unsigned int; // GLint;

	using ColorRGB = math::vec3f;
	using ColorRGBA = math::vec4f;

	using Color = ColorRGB;

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
	enum class TextureFlags
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

		ColorDepth = (Color | Depth),
	};

	enum class ContextFlags
	{
		None             = (1 << 0),
		DepthTest        = (1 << 1),

		Default          = DepthTest,
	};

	using uniform_t = std::variant<int, float, bool, math::Vector2D, math::Vector3D, math::Vector4D, math::Matrix2x2, math::Matrix3x3, math::Matrix4x4>;

	// Bitfield boilerplate:
	inline std::uint32_t operator& (BufferType a, BufferType b) { return (std::uint32_t)((std::uint32_t)a & (std::uint32_t)b); }
	inline BufferType operator~ (BufferType a) { return (BufferType)~(std::uint32_t)a; }
	inline BufferType operator| (BufferType a, BufferType b) { return (BufferType)((std::uint32_t)a | (std::uint32_t)b); }
	inline BufferType operator^ (BufferType a, BufferType b) { return (BufferType)((std::uint32_t)a ^ (std::uint32_t)b); }
	inline BufferType& operator|= (BufferType& a, BufferType b) { return (BufferType&)((std::uint32_t&)a |= (std::uint32_t)b); }
	inline BufferType& operator&= (BufferType& a, BufferType b) { return (BufferType&)((std::uint32_t&)a &= (std::uint32_t)b); }
	inline BufferType& operator^= (BufferType& a, BufferType b) { return (BufferType&)((std::uint32_t&)a ^= (std::uint32_t)b); }

	inline std::uint32_t operator& (TextureFlags a, TextureFlags b) { return (std::uint32_t)((std::uint32_t)a & (std::uint32_t)b); }
	inline TextureFlags operator~ (TextureFlags a) { return (TextureFlags)~(std::uint32_t)a; }
	inline TextureFlags operator| (TextureFlags a, TextureFlags b) { return (TextureFlags)((std::uint32_t)a | (std::uint32_t)b); }
	inline TextureFlags operator^ (TextureFlags a, TextureFlags b) { return (TextureFlags)((std::uint32_t)a ^ (std::uint32_t)b); }
	inline TextureFlags& operator|= (TextureFlags& a, TextureFlags b) { return (TextureFlags&)((std::uint32_t&)a |= (std::uint32_t)b); }
	inline TextureFlags& operator&= (TextureFlags& a, TextureFlags b) { return (TextureFlags&)((std::uint32_t&)a &= (std::uint32_t)b); }
	inline TextureFlags& operator^= (TextureFlags& a, TextureFlags b) { return (TextureFlags&)((std::uint32_t&)a ^= (std::uint32_t)b); }

	inline std::uint32_t operator& (ContextFlags a, ContextFlags b) { return (std::uint32_t)((std::uint32_t)a & (std::uint32_t)b); }
	inline ContextFlags operator~ (ContextFlags a) { return (ContextFlags)~(std::uint32_t)a; }
	inline ContextFlags operator| (ContextFlags a, ContextFlags b) { return (ContextFlags)((std::uint32_t)a | (std::uint32_t)b); }
	inline ContextFlags operator^ (ContextFlags a, ContextFlags b) { return (ContextFlags)((std::uint32_t)a ^ (std::uint32_t)b); }
	inline ContextFlags& operator|= (ContextFlags& a, ContextFlags b) { return (ContextFlags&)((std::uint32_t&)a |= (std::uint32_t)b); }
	inline ContextFlags& operator&= (ContextFlags& a, ContextFlags b) { return (ContextFlags&)((std::uint32_t&)a &= (std::uint32_t)b); }
	inline ContextFlags& operator^= (ContextFlags& a, ContextFlags b) { return (ContextFlags&)((std::uint32_t&)a ^= (std::uint32_t)b); }
}