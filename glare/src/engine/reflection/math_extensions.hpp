#pragma once

#include "common_extensions.hpp"

#include <engine/meta/hash.hpp>

#include <math/types.hpp>
#include <math/conversion.hpp>

#include <utility>
#include <type_traits>
#include <string_view>
#include <string>
#include <cstdint>

namespace engine
{
    namespace impl
    {
        // General:
        template <typename LeftType, typename RightType = LeftType, typename OutType = decltype(std::declval<LeftType>() + std::declval<RightType>())>
        OutType add_impl(const LeftType& left, const RightType& right)
        {
            return (left + right);
        }

        template <typename LeftType, typename RightType = LeftType, typename OutType = decltype(std::declval<LeftType>() - std::declval<RightType>())>
        OutType subtract_impl(const LeftType& left, const RightType& right)
        {
            return (left - right);
        }

        template <typename LeftType, typename RightType = LeftType, typename OutType = decltype(std::declval<LeftType>()* std::declval<RightType>())>
        OutType multiply_impl(const LeftType& left, const RightType& right)
        {
            return (left * right);
        }

        template <typename LeftType, typename RightType = LeftType, typename OutType = decltype(std::declval<LeftType>() / std::declval<RightType>())>
        OutType divide_impl(const LeftType& left, const RightType& right)
        {
            return (left / right);
        }

        template <typename MatrixType>
        MatrixType inverse_impl(const MatrixType& m)
        {
            return glm::inverse(m);
        }

        template <typename MatrixType>
        MatrixType transpose_impl(const MatrixType& m)
        {
            return glm::transpose(m);
        }

        template <typename VectorType>
        VectorType normalize_impl(const VectorType& v)
        {
            return glm::normalize(v);
        }

        // Vector:
        template <typename T, typename OutputType=T>
        OutputType vector_operator_unary_plus_impl(const T& value)
        {
            using value_t = typename T::value_type;
            using output_value_t = typename OutputType::value_type;

            constexpr auto vector_length = T::length();

            if constexpr (vector_length == 2)
            {
                return OutputType
                {
                    operator_unary_plus_impl<value_t, output_value_t>(value.x),
                    operator_unary_plus_impl<value_t, output_value_t>(value.y)
                };
            }
            else if constexpr (vector_length == 3)
            {
                return OutputType
                {
                    operator_unary_plus_impl<value_t, output_value_t>(value.x),
                    operator_unary_plus_impl<value_t, output_value_t>(value.y),
                    operator_unary_plus_impl<value_t, output_value_t>(value.z)
                };
            }
            else if constexpr (vector_length == 4)
            {
                return OutputType
                {
                    operator_unary_plus_impl<value_t, output_value_t>(value.x),
                    operator_unary_plus_impl<value_t, output_value_t>(value.y),
                    operator_unary_plus_impl<value_t, output_value_t>(value.z),
                    operator_unary_plus_impl<value_t, output_value_t>(value.w)
                };
            }
            else
            {
                static_assert(std::integral_constant<T, false>::value, "Unsupported vector type.");
            }
        }

        template <typename T, typename OutputType=T>
        OutputType vector_operator_unary_minus_impl(const T& value)
        {
            using value_t = typename T::value_type;
            using output_value_t = typename OutputType::value_type;

            constexpr auto vector_length = T::length();

            if constexpr (vector_length == 2)
            {
                return OutputType
                {
                    operator_unary_minus_impl<value_t, output_value_t>(value.x),
                    operator_unary_minus_impl<value_t, output_value_t>(value.y)
                };
            }
            else if constexpr (vector_length == 3)
            {
                return OutputType
                {
                    operator_unary_minus_impl<value_t, output_value_t>(value.x),
                    operator_unary_minus_impl<value_t, output_value_t>(value.y),
                    operator_unary_minus_impl<value_t, output_value_t>(value.z)
                };
            }
            else if constexpr (vector_length == 4)
            {
                return OutputType
                {
                    operator_unary_minus_impl<value_t, output_value_t>(value.x),
                    operator_unary_minus_impl<value_t, output_value_t>(value.y),
                    operator_unary_minus_impl<value_t, output_value_t>(value.z),
                    operator_unary_minus_impl<value_t, output_value_t>(value.w)
                };
            }
            else
            {
                static_assert(std::integral_constant<T, false>::value, "Unsupported vector type.");
            }
        }

        template <typename T, typename IndexType=typename T::length_type, typename OutputType=typename T::value_type> // std::size_t
        OutputType vector_operator_index_subscript_impl(const T& self, IndexType index)
        {
            if constexpr (std::is_signed_v<IndexType>)
            {
                if (index < 0)
                {
                    return OutputType {};
                }
            }

            if (index >= T::length())
            {
                return OutputType {};
            }

            return static_cast<OutputType>(self[index]);
        }

        template <typename T, typename OutputType=typename T::value_type>
        OutputType vector_operator_string_view_subscript_impl(const T& self, std::string_view query_str)
        {
            using IndexType = typename T::length_type;

            const auto query_id = hash(query_str);

            switch (query_id)
            {
                case "x"_hs:
                case "X"_hs:
                case "r"_hs:
                case "R"_hs:
                    return vector_operator_index_subscript_impl<T, IndexType, OutputType>(self, 0);

                case "y"_hs:
                case "Y"_hs:
                case "g"_hs:
                case "G"_hs:
                    return vector_operator_index_subscript_impl<T, IndexType, OutputType>(self, 1);

                case "z"_hs:
                case "Z"_hs:
                case "b"_hs:
                case "B"_hs:
                    return vector_operator_index_subscript_impl<T, IndexType, OutputType>(self, 2);

                case "w"_hs:
                case "W"_hs:
                case "a"_hs:
                case "A"_hs:
                    return vector_operator_index_subscript_impl<T, IndexType, OutputType>(self, 3);
            }

            return OutputType {};
        }

        template <typename T, typename OutputType = typename T::value_type>
        OutputType vector_operator_string_subscript_impl(const T& self, const std::string& query_str)
        {
            return vector_operator_string_view_subscript_impl(self, std::string_view { query_str });
        }

        inline math::Vector4D vec4_from_vec2(const math::Vector2D& xy, float z, float w)
        {
            return { xy.x, xy.y, z, w };
        }

        inline math::Vector4D vec4_from_vec3(const math::Vector3D& xyz, float w)
        {
            return { xyz.x, xyz.y, xyz.z, w };
        }

        inline math::Vector3D vec3_from_vec2(const math::Vector2D& xy, float z)
        {
            return { xy.x, xy.y, z };
        }

        // Quaternion:
        inline math::Vector4D quat_vec4_multiply_impl(const math::Quaternion& q, const math::Vector4D& v)
        {
            return (q * v);
        }

        inline math::Vector3D quat_vec3_multiply_impl(const math::Quaternion& q, const math::Vector3D& v)
        {
            //auto vec4_result = quat_vec4_multiply_impl(q, vec4_from_vec3(v, 1.0f));
            //return { vec4_result.x, vec4_result.y, vec4_result.z };

            return (q * v);
        }

        // Matrix:
        template <typename MatrixType>
        MatrixType mat_quat_multiply_impl(const MatrixType& m, const math::Quaternion& q)
        {
            return (m * math::quaternion_to_matrix<MatrixType>(q));
        }

        inline math::Vector4D mat4_vec4_multiply_impl(const math::Matrix4x4& m, const math::Vector4D& v)
        {
            return (m * v);
        }

        inline math::Vector3D mat4_vec3_multiply_impl(const math::Matrix4x4& m, const math::Vector3D& v)
        {
            const auto vec4_result = mat4_vec4_multiply_impl(m, vec4_from_vec3(v, 1.0f));

            return { vec4_result.x, vec4_result.y, vec4_result.z };
        }

        inline math::Matrix4x4 mat4_translate_impl(const math::Matrix4x4& m, const math::Vector3D& v)
        {
            return glm::translate(m, v);
        }
    }
}