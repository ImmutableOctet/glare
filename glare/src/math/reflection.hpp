#pragma once

// This header provides interoperative facilities for `entt`'s meta-type reflection system.

#include "types.hpp"

#include <util/reflection.hpp>

#include <type_traits>

// TODO: Implement reflection for matrices.
// (May end up doing identity functions + acknowledging types, etc.)

// TODO: Ensure `x`, `y` and `z` members are reflected appropriately for vector types.
// (May require wrapper functions)

namespace math
{
    namespace impl
    {
        // Shorthand for `entt::meta` with a custom identifier.
        template <typename ReflectedType, typename IdentifierType>
        auto reflect_type(IdentifierType&& id)
        {
            return entt::meta<ReflectedType>().type(id);
        }

        template <typename VectorType>
        VectorType make_vector2d(float x, float y) // typename VectorType::type
        {
            return { x, y };
        }

        template <>
        inline Vector3D make_vector2d<Vector3D>(float x, float y)
        {
            return { x, y, 0.0f };
        }

        template <>
        inline Vector4D make_vector2d<Vector4D>(float x, float y)
        {
            return { x, y, 0.0f, 0.0f }; // 1.0f
        }

        template <typename VectorType>
        VectorType make_vector3d(float x, float y, float z) // typename VectorType::type
        {
            return { x, y, z };
        }

        template <>
        Vector4D make_vector3d<Vector4D>(float x, float y, float z) // typename VectorType::type
        {
            return { x, y, z, 0.0f }; // 1.0f
        }

        template <typename VectorType>
        VectorType make_vector4d(float x, float y, float z, float w) // typename VectorType::type
        {
            return { x, y, z, w };
        }

        // Registers members named `x` and `y` from `VectorType`.
        template <typename VectorType, typename IdentifierType>
        auto reflect_vector2d(IdentifierType&& id)
        {
            using namespace engine::literals;

            return reflect_type<VectorType>(id)
                .data<&VectorType::x>("x"_hs)
                .data<&VectorType::y>("y"_hs)

                .ctor<&make_vector2d<VectorType>>()
            ;
        }

        template <typename VectorType, typename IdentifierType>
        auto reflect_vector3d(IdentifierType&& id)
        {
            using namespace engine::literals;

            return reflect_vector2d<VectorType>(id)
                .data<&VectorType::z>("z"_hs)

                .ctor<&make_vector3d<VectorType>>()
            ;
        }

        template <typename VectorType, typename IdentifierType>
        auto reflect_vector4d(IdentifierType&& id)
        {
            using namespace engine::literals;

            return reflect_vector3d<VectorType>(id)
                .data<&VectorType::w>("w"_hs)

                .ctor<&make_vector4d<VectorType>>()
            ;
        }
    }

    // Reflects type `T` using `id`.
    // By default, reflection of types is not supported.
    // Each reflected type needs its own implementation specialized from this
    template <typename T, typename IdentifierType>
    auto reflect(IdentifierType&& id)
    {
        using namespace engine::literals;

        if constexpr (std::is_same_v<T, Vector2D>)
        {
            return impl::reflect_vector2d<T>(id);
        }
        else if constexpr (std::is_same_v<T, Vector3D>)
        {
            return impl::reflect_vector3d<T>(id)
                .data<&T::r>("r"_hs)
                .data<&T::g>("g"_hs)
                .data<&T::b>("b"_hs)
            ;
        }
        else if constexpr (std::is_same_v<T, Vector4D>)
        {
            return impl::reflect_vector4d<T>(id)
                .data<&T::r>("r"_hs)
                .data<&T::g>("g"_hs)
                .data<&T::b>("b"_hs)
                .data<&T::a>("a"_hs)
            ;
        }
        else if constexpr (std::is_same_v<T, vec2i>)
        {
            return impl::reflect_vector2d<T>(id);
        }
        else
        {
            static_assert(std::integral_constant<T, false>::value, "Reflection is not currently supported for this type.");
        }
    }

    /*
        Reflects `T` under its default hash/identifier.

        NOTES:
            * This does not guarantee `T` will be associated to the `math` namespace.
        
            * This overload may not be ideal for all use-cases.
            For this reason, the manual-identifier options are provided.
    */
    template <typename T>
    auto reflect()
    {
        return reflect<T>(entt::type_hash<T>::value());
    }
}