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

		// Specialized:

		// Only used for special-case scenarios, such as 24-bit depth buffers, etc.
		UInt24,

		// Only used for 32-bit depth + 8-bit stencil. (32 + 8)
		Int32_8,

		Bit,    // 1-bit integer.
		Nibble, // 4-bit integer.

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
	enum class TextureFlags : std::uint32_t
	{
		None               = (1 << 0),

		// TODO: Review the best way to handle depth and stencil data:
		DepthMap           = (1 << 1),
		DepthStencilMap    = (1 << 2),

		LinearFiltering    = (1 << 3),
		MipMap             = (1 << 4),
		WrapS              = (1 << 5),
		WrapT              = (1 << 6),

		// This flag currently does nothing, it simply states that a texture was a result of a custom allocation.
		Dynamic            = (1 << 7),
		
		WrapST             = (WrapS|WrapT),

		Default            = (LinearFiltering|WrapST),
	}; FLAG_ENUM(std::uint32_t, TextureFlags);

	enum class BufferType : std::uint32_t
	{
		Color = (1 << 0),
		Depth = (1 << 1),

		ColorDepth = (Color | Depth),
	}; FLAG_ENUM(std::uint32_t, BufferType);

	enum class ContextFlags : std::uint32_t
	{
		None             = (1 << 0),
		DepthTest        = (1 << 1),
		FaceCulling      = (1 << 2),

		Default          = (DepthTest|FaceCulling),
	}; FLAG_ENUM(std::uint32_t, ContextFlags);

	enum class TextureFormat
	{
		Unknown = 0,

		R,
		RG,
		RGB,
		RGBA,

		// Context sensitive; equivalent to the driver's default depth-format.
		Depth,

		// Context sensitive; equivalent to the driver's default stencil-format.
		Stencil,

		// Context sensitive; equivalent to a depth + stencil buffer.
		DepthStencil,
	};

	// Types of integrated render buffers.
	enum class RenderBufferType
	{
		Color,
		Depth,
		Stencil,
		DepthStencil,

		Unknown,
	};

	using uniform_t = std::variant<int, float, bool, math::Vector2D, math::Vector3D, math::Vector4D, math::Matrix2x2, math::Matrix3x3, math::Matrix4x4>;
}