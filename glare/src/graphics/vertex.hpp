#pragma once

#include <typeinfo>
#include <string>
#include <array>
#include <algorithm>
#include <cstddef>

///#include <half/half.hpp>

#include "types.hpp"
#include <util/algorithm.hpp>
#include <math/math.hpp>

#define DECLARE_ATTRIBUTE(vertex_type, member) Vertex::get_attribute<decltype(member)>(offsetof(vertex_type, member)) // Vertex::get_attribute

#define VERTEX_ATTRIBUTE(name, member_type, member_name)                     \
template <typename VertexType>                                               \
struct Attribute_##name : public VertexType                                  \
{                                                                            \
	member_type member_name;                                                 \
                                                                             \
	inline static auto format()                                              \
	{                                                                        \
		return util::concatenate                                             \
		(                                                                    \
			VertexType::format(),                                            \
			std::array { DECLARE_ATTRIBUTE(Attribute_##name, member_name) }  \
		);                                                                   \
	}                                                                        \
};                                                                           \
                                                                             \
template <typename VertexType>                                               \
using A_##name = Attribute_##name<VertexType>;                               \

#define VERTEX_FIELD(name, type) VERTEX_ATTRIBUTE(name, type, name)

namespace graphics
{
	struct Vertex
	{
		using Format = VertexAttribute;

		math::vec3f position;

		//Vertex() = default;
		//Vertex(const Vertex&) = default;

		// Retrieves the primitive 'ElementType' assocaited with 'T'.
		template <typename T>
		inline static ElementType GetElementType() noexcept
		{
			const auto& type = typeid(T); // constexpr

			// Primitives:
			if (type == typeid(std::int8_t))           return ElementType::Byte;
			else if (type == typeid(std::uint8_t))     return ElementType::UByte;
			else if (type == typeid(std::int16_t))     return ElementType::Short;
			else if (type == typeid(std::uint16_t))    return ElementType::UShort;
			else if (type == typeid(std::int32_t))     return ElementType::Int;
			else if (type == typeid(std::uint32_t))    return ElementType::UInt;
			else if (type == typeid(float))            return ElementType::Float;
			else if (type == typeid(double))           return ElementType::Double;

			// Extensions:
			///else if (type == typeid(half_float::half)) return ElementType::Half;
			else if (type == typeid(std::string))      return ElementType::Char;

			// Math:
			else if (type == typeid(math::vec2f))      return ElementType::Float;
			else if (type == typeid(math::vec3f))      return ElementType::Float;
			else if (type == typeid(math::vec4f))      return ElementType::Float;

			else if (type == typeid(math::vec2i))      return ElementType::Int;
			else if (type == typeid(math::vec3i))      return ElementType::Int;
			else if (type == typeid(math::vec4i))      return ElementType::Int;

			else if (type == typeid(math::mat2f))      return ElementType::Float;
			else if (type == typeid(math::mat3f))      return ElementType::Float;
			else if (type == typeid(math::mat4f))      return ElementType::Float;
			
			// Unsupported:
			else return ElementType::Unknown;
		}

		template <typename T>
		inline static int get_num_elements()
		{
			const auto& type = typeid(T);

			// Primitives:
			if (type == typeid(std::int8_t)) return 1;
			else if (type == typeid(std::uint8_t)) return 1;
			else if (type == typeid(std::int16_t)) return 1;
			else if (type == typeid(std::uint16_t)) return 1;
			else if (type == typeid(std::int32_t)) return 1;
			else if (type == typeid(std::uint32_t)) return 1;
			else if (type == typeid(float)) return 1;
			else if (type == typeid(double)) return 1;

			// Extensions:
			///else if (type == typeid(half_float::half)) return 1;

			// Math:
			else if (type == typeid(math::vec2f)) return 2;
			else if (type == typeid(math::vec3f)) return 3;
			else if (type == typeid(math::vec4f)) return 4;

			else if (type == typeid(math::vec2i)) return 2;
			else if (type == typeid(math::vec3i)) return 3;
			else if (type == typeid(math::vec4i)) return 4;

			else if (type == typeid(math::mat2f)) return 2*2;
			else if (type == typeid(math::mat3f)) return 3*3;
			else if (type == typeid(math::mat4f)) return 4*4;

			// Unsupported:
			else return 0;
		}

		template <typename T>
		inline static VertexAttribute get_attribute(int offset=0) // get_static_format
		{
			return { GetElementType<T>(), get_num_elements<T>(), offset };
		}

		inline static auto format()
		{
			return std::array { DECLARE_ATTRIBUTE(Vertex, position) };
		}
	};

	// Attribute templates:
	VERTEX_ATTRIBUTE(RGB, ColorRGB, color);
	VERTEX_ATTRIBUTE(TexCoords, math::vec2f, uv);
	VERTEX_ATTRIBUTE(Normal, math::vec3f, normal);
	VERTEX_ATTRIBUTE(Tangent, math::vec3f, tangent);
	VERTEX_ATTRIBUTE(Bitangent, math::vec3f, bitangent);

	VERTEX_ATTRIBUTE(BoneIndices, math::vec4f, bone_indices); //VERTEX_ATTRIBUTE(BoneIndices, math::vec4i, bone_indices);

	VERTEX_ATTRIBUTE(BoneWeights, math::vec4f, bone_weights);

	// Adds 4 bone indices and 4 weight channels to the 'VertexType' specified.
	template <typename VertexType>
	using AnimatedVertex = A_BoneWeights<A_BoneIndices<VertexType>>;

	//using PositionVertex = Vertex;
	using SimpleVertex = Vertex;
	using TexturedVertex = A_TexCoords<Vertex>;
	using StandardVertex = A_Bitangent<A_Tangent<A_TexCoords<A_Normal<Vertex>>>>;
	using StandardAnimationVertex = AnimatedVertex<StandardVertex>;

	static constexpr unsigned int VERTEX_MAX_BONE_INFLUENCE = 4;

	int get_next_weight_channel(const StandardAnimationVertex& vertex);
}