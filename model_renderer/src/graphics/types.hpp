#pragma once

#include <types.hpp>
#include <math/math.hpp>

#include <utility>

#include <string_view>
#include <string>
#include <map>
#include <unordered_map>
#include <type_traits>
#include <variant>

/*
inline bool operator<(const std::string& a, const std::string_view& b) noexcept
{
	return (a < b);
}
*/

inline bool operator<(const std::string_view& a, const std::string& b) noexcept
{
	return (a < b);
}

namespace engine
{
	class ResourceManager;
}

namespace graphics
{
	using ResourceManager = engine::ResourceManager;

	class Texture;
	class Material;

	using NativeContext = void*;
	using ContextHandle = unsigned int; // GLint;

	using ColorRGB = math::vec3f;
	using ColorRGBA = math::vec4f;

	using Color = ColorRGB;

	// TODO: Implement half-size indices for smaller meshes.
	using MeshIndex = std::uint32_t; // GLuint;

	struct Point
	{
		union
		{
			struct { int x, y; };
			struct { int width, height; };
		};
	};

	struct PointRect
	{
		Point start;

		union
		{
			Point end;
			struct { int width, height; };
		};
	};

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

	enum class VertexWinding
	{
		Clockwise,
		CounterClockwise,
	};

	enum class TextureType : std::int32_t // int
	{
		StartType = 1,

		None = 0,
		Diffuse,
		Specular,
		Ambient,
		Emissive,
		Height,
		Normals,
		Shininess,
		Opacity,
		Displacement,
		Lightmap,
		Reflection,
		BaseColor,
		NormalCamera,
		EmissionColor,
		Metalness,
		DiffuseRoughness,
		AmbientOcclusion,
		Unknown,

		MaxTypes,
		EndType = MaxTypes
	};

	struct VertexAttribute
	{
		ElementType type;

		int num_elements; // unsigned int
		int offset = 0;
	};

	enum class FrameBufferType : std::uint32_t
	{
		Unknown = (1 << 0),
		Read    = (1 << 1),
		Write   = (1 << 2),

		ReadWrite = (Read | Write),
	}; FLAG_ENUM(std::uint32_t, FrameBufferType);

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
		VSync            = (1 << 3),

		Default          = (DepthTest|FaceCulling|VSync),
	}; FLAG_ENUM(std::uint32_t, ContextFlags);

	enum class CanvasDrawMode : std::uint32_t
	{
		None = (1 << 0),

		Opaque         = (1 << 1),
		Transparent    = (1 << 2),

		// If enabled, the active shader will not be checked before drawing.
		IgnoreShaders  = (1 << 3),

		All = (Opaque | Transparent),
	}; FLAG_ENUM(std::uint32_t, CanvasDrawMode);

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

	using TextureArray = std::vector<ref<Texture>>;
	using TextureGroup = std::variant<ref<Texture>, TextureArray>; // Used to represent a single texture object or vector of textures objects. (Represents a 'TextureType')

	// Map of string-identifiers to 'TextureGroup' objects.
	using TextureMap   = std::unordered_map<std::string, TextureGroup>; // string_view

	using UniformData  = std::variant<bool, int, float, ContextHandle, math::Vector2D, math::Vector3D, math::Vector4D, math::Matrix2x2, math::Matrix3x3, math::Matrix4x4>;
	using UniformMap   = std::map<std::string, UniformData, std::less<>>; // std::string_view // unordered_map

	template <typename fn_type>
	void enumerate_texture_types(fn_type&& fn)
	{
		for (auto type = static_cast<int>(TextureType::StartType); type < static_cast<decltype(type)>(TextureType::EndType); type++)
		{
			auto texture_type = static_cast<TextureType>(type);

			fn(texture_type);
		}
	}
}