#pragma once

#include <math/types.hpp>
#include <math/conversion.hpp>

#include <utility>

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