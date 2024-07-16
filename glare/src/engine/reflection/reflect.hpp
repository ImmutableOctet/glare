#pragma once

#include "reflect_decl.hpp"
#include "math_decl.hpp"

#include "aggregate.hpp"
#include "enum.hpp"

#include <engine/meta/traits.hpp>

#include <entt/meta/meta.hpp>

#include <type_traits>

#define GLARE_IMPL_DEFINE_REFLECT()                     \
    template <typename T>                               \
    void reflect() { impl::reflect_default_impl<T>(); }

namespace engine
{
    namespace impl
    {
        template <typename T=void>
        void reflect_default_impl()
        {
            if constexpr (!std::is_same_v<T, void>)
            {
                if constexpr (std::is_enum_v<T>)
                {
                    engine::reflect_enum<T>();
                }
                else if constexpr (has_function_reflect_v<T, void>)
                {
                    T::reflect();
                }
                else if constexpr (has_function_reflect_v<T, entt::meta_factory<T>>)
                {
                    // TODO: Determine if it makes sense to forward the return-value to the caller.
                    T::reflect();
                }
#if GLARE_AGGREGATE_REFLECTION_SUPPORTED
                else if constexpr (util::is_pfr_reflective_v<T>)
                {
                    reflect_aggregate_fields<T>();
                }
#endif // GLARE_AGGREGATE_REFLECTION_SUPPORTED
                else
                {
                    static_assert(std::integral_constant<T, false>::value, "Reflection definition missing for type `T`.");
                }
            }
        }
    }

    GLARE_IMPL_DEFINE_REFLECT();
}