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
            //return make_vector3d(x, y, 0.0f);
            return { x, y, 0.0f };
        }

        template <typename VectorType>
        VectorType make_vector3d(float x, float y, float z) // typename VectorType::type
        {
            return { x, y, z };
        }

        // Registers members named `x` and `y` from `VectorType`.
        template <typename VectorType, typename IdentifierType>
        auto reflect_vector2d(IdentifierType&& id)
        {
            using namespace entt::literals;

            return reflect_type<VectorType>(id)
                .data<&VectorType::x>("x"_hs)
                .data<&VectorType::y>("y"_hs)

                /*
                .ctor
                <
                    float, // typename VectorType::type
                    float  // typename VectorType::type
                >()
                */

                .ctor<&make_vector2d<VectorType>>()
            ;
        }

        template <typename VectorType, typename IdentifierType>
        auto reflect_vector3d(IdentifierType&& id)
        {
            using namespace entt::literals;

            return reflect_vector2d<VectorType>(id)
                .data<&VectorType::z>("z"_hs)

                /*
                .ctor
                <
                    float, // typename VectorType::type,
                    float, // typename VectorType::type,
                    float  // typename VectorType::type
                >()
                */

                .ctor<&make_vector3d<VectorType>>()
            ;
        }
    }

    // Reflects type `T` using `id`.
    // By default, reflection of types is not supported.
    // Each reflected type needs its own implementation specialized from this
    template <typename T, typename IdentifierType>
    auto reflect(IdentifierType&& id)
    {
        using namespace entt::literals;

        if constexpr (std::is_same_v<T, Vector2D>)
        {
            return impl::reflect_vector2d<T>(id);
        }
        else if constexpr (std::is_same_v<T, Vector3D>)
        {
            return impl::reflect_vector3d<T>(id);
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