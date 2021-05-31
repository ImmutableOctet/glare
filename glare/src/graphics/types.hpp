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
#include <tuple>

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

	// TODO: Implement half-size indices for smaller meshes.
	using MeshIndex = std::uint32_t; // GLuint;

	/*
	struct Point
	{
		union
		{
			struct { int x, y; };
			//struct { int width, height; };
		};
	};
	*/

	using Point = math::vec2i; // math::Vector2D;

	struct PointRect
	{
		//using Type = float;
		using Type = int;

		Point start = {};
		Point end = {};

		void set_width(Type value);
		void set_height(Type value);

		inline void set_size(Type width, Type height)
		{
			set_width(width);
			set_height(height);
		}

		inline void set_size(const Point& size)
		{
			set_size(size.x, size.y);
		}

		inline Type get_x() const
		{
			return start.x;
		}

		inline Type get_y() const
		{
			return start.y;
		}

		Type get_width() const; // inline auto
		Type get_height() const; // inline auto
		Type get_length() const; // inline auto
	};

	using Viewport = PointRect;

	enum class GBufferFlags : std::uint32_t
	{
		None = 0,
		
		DepthTexture = (1 << 0),
		Position     = (1 << 1),

		Default = DepthTexture // | Position
	};

	FLAG_ENUM(std::uint32_t, GBufferFlags);

	enum class ShaderType
	{
		Vertex,
		Fragment,
		Geometry,
		Tessellation
	};

	class ShaderSource
	{
		public:
			using StringType = std::string;

			static StringType load_source_string(const std::string& path, bool check_for_version=true, std::string* version_out=nullptr); // false

			// Implementation found in "shader.cpp".
			static ShaderSource Load(const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path={}); // StringType

			StringType vertex;
			StringType fragment;
			StringType geometry;

			StringType version;
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

	struct VertexAttribute
	{
		ElementType type;

		int num_elements; // unsigned int
		int offset = 0;
	};

	enum class TextureType
	{
		Texture2D = 0,

		CubeMap,
		//Texture3D,

		Normal  = Texture2D,
		Default = Texture2D,
	};

	enum class TextureClass : std::int32_t // int
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

	// NOTE: Support for texture flags is driver-dependent.
	enum class TextureFlags : std::uint32_t
	{
		None               = (1 << 0),

		// TODO: Review the best way to handle depth and stencil data:
		//DepthMap           = (1 << 1),
		//DepthStencilMap    = (1 << 2),
		Clamp = (1 << 2),

		LinearFiltering    = (1 << 3),
		MipMap             = (1 << 4),
		WrapS              = (1 << 5),
		WrapT              = (1 << 6),

		Dynamic            = (1 << 7),
		
		WrapST             = (WrapS|WrapT),

		Default            = (LinearFiltering|WrapST),
	};
	
	FLAG_ENUM(std::uint32_t, TextureFlags);

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

	enum class FrameBufferType : std::uint32_t
	{
		Unknown = (1 << 0),
		Read    = (1 << 1),
		Write   = (1 << 2),

		ReadWrite = (Read | Write),
	}; FLAG_ENUM(std::uint32_t, FrameBufferType);

	enum class BufferType : std::uint32_t
	{
		Color = (1 << 0),
		Depth = (1 << 1),

		ColorDepth = (Color | Depth),
	};
	
	FLAG_ENUM(std::uint32_t, BufferType);

	enum class ContextFlags : std::uint32_t
	{
		None             = (1 << 0),
		DepthTest        = (1 << 1),
		FaceCulling      = (1 << 2),
		VSync            = (1 << 3),

		Default          = (DepthTest|FaceCulling|VSync),
	};
	
	FLAG_ENUM(std::uint32_t, ContextFlags);

	enum class CanvasDrawMode : std::uint32_t
	{
		None            = (1 << 0),

		Opaque          = (1 << 1),
		Transparent     = (1 << 2),

		// If enabled, the active shader will not be checked before drawing.
		IgnoreShaders   = (1 << 3),
		IgnoreTextures  = (1 << 4),
		IgnoreMaterials = (1 << 5),

		// When rendering a scene normally, this disables receiving shadows.
		IgnoreShadows   = (1 << 6),

		_Shadow         = (1 << 7),
		Shadow          = _Shadow | (Opaque|IgnoreShaders|IgnoreTextures|IgnoreMaterials), // Transparent
		//Shadow        = (1 << 7),

		All = (Opaque | Transparent),
	};
	
	FLAG_ENUM(std::uint32_t, CanvasDrawMode);

	// Types of integrated render buffers.
	enum class RenderBufferType
	{
		Color,
		Depth,
		Stencil,
		DepthStencil,

		Unknown,
	};

	using Vector = math::Vector;
	using Vector2D = math::Vector2D;
	using Matrix = math::Matrix; // 4x4

	using VectorArray = std::vector<Vector>;
	using MatrixArray = std::vector<Matrix>;
	using FloatArray = std::vector<float>;
	using FloatValues = std::variant<float*, FloatArray*>;

	using LightPositions = std::variant<Vector*, VectorArray*>; // const
	using LightMatrices  = std::variant<Matrix*, MatrixArray*>;

	using TextureArray    = std::vector<ref<Texture>>;

	// Names are owning views.
	using NamedTextureArray = std::vector<std::tuple<std::string, ref<Texture>>>; // string_view

	using TextureArrayRaw = std::vector<const Texture*>;

	// Names are non-owning views.
	//using NamedTextureArrayRaw = std::vector<std::tuple<std::string_view, const Texture*>>;
	using NamedTextureArrayRaw = std::unordered_map<std::string, std::vector<const Texture*>>;

	using TextureGroup    = std::variant<ref<Texture>, TextureArray>; // Used to represent a single texture object or vector of textures objects. (Represents a 'TextureClass')
	//using NamedTextureGroup = std::tuple<std::string, TextureGroup>;

	using TextureGroupRaw = std::variant<const Texture*, TextureArrayRaw*>;

	//using NamedTextureGroupRaw = std::tuple<std::string, TextureGroup>;
	using NamedTextureGroupRaw = std::variant<std::tuple<std::string_view, const Texture*>, const NamedTextureArrayRaw*>;

	// Map of string-identifiers to 'TextureGroup' objects.
	using TextureMap   = std::unordered_map<std::string, TextureGroup>; // string_view

	using UniformData  = std::variant
	<
		bool, int, float, ContextHandle,
		math::Vector2D, math::Vector3D, math::Vector4D,
		math::Matrix2x2, math::Matrix3x3, math::Matrix4x4,
			
		graphics::VectorArray,
		graphics::MatrixArray,
		graphics::FloatArray
	>;

	using UniformMap   = std::map<std::string, UniformData, std::less<>>; // std::string_view // unordered_map

	template <typename fn_type>
	void enumerate_texture_types(fn_type&& fn)
	{
		for (auto type = static_cast<int>(TextureClass::StartType); type < static_cast<decltype(type)>(TextureClass::EndType); type++)
		{
			auto texture_type = static_cast<TextureClass>(type);

			fn(texture_type);
		}
	}
}