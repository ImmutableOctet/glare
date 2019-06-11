#pragma once

#include <typeinfo>
#include <string>
#include <array>
#include <algorithm>
#include <cstddef>

#include <half/half.hpp>

#include "types.hpp"
#include <util/algorithm.hpp>
#include <math/math.hpp>

#define VERTEX_ATTRIBUTE(vertex_type, member) get_attribute<decltype(member)>(offsetof(vertex_type, member)) // Vertex::get_attribute

namespace graphics
{
	struct Vertex
	{
		using Format = VertexAttribute;

		math::vec3f position;

		// Retrieves the primitive 'ElementType' assocaited with 'T'.
		template <typename T>
		inline static ElementType GetElementType() noexcept
		{
			const auto& type = typeid(T);

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
			else if (type == typeid(half_float::half)) return ElementType::Half;
			else if (type == typeid(std::string))      return ElementType::Char;

			// Math:
			else if (type == typeid(math::vec2f))      return ElementType::Float;
			else if (type == typeid(math::vec3f))      return ElementType::Float;
			else if (type == typeid(math::vec4f))      return ElementType::Float;
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
			else if (type == typeid(half_float::half)) return 1;

			// Math:
			else if (type == typeid(math::vec2f)) return 2;
			else if (type == typeid(math::vec3f)) return 3;
			else if (type == typeid(math::vec4f)) return 4;
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
			return std::array { VERTEX_ATTRIBUTE(Vertex, position) };
		}
	};

	struct RGBVertex : public Vertex
	{
		math::vec3f color;
	};

	struct TextureVertex : public Vertex
	{
		math::vec2f uv;

		inline static auto format()
		{
			return util::concatenate<VertexAttribute, 1, 1>(Vertex::format(), { VERTEX_ATTRIBUTE(TextureVertex, uv) });
		}
	};
}