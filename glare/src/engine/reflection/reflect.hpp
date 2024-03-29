#pragma once

#if GLARE_BOOST_PFR_ENABLED
    #include "aggregate.hpp"
#endif

#include <engine/meta/traits.hpp>

#include <entt/meta/meta.hpp>

#include <type_traits>

namespace engine
{
    // NOTE: In the default case of `T=void`, the overridden version of this template is used.
    // TODO: Look into best way to handle multiple calls to reflect. (This is currently only managed in `reflect_all`)
    template <typename T=void>
    void reflect()
    {
        if constexpr (std::is_enum_v<T>)
        {
            reflect_enum<T>();
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
#if GLARE_BOOST_PFR_ENABLED
        else if constexpr (util::is_pfr_reflective_v<T>)
        {
            reflect_aggregate_fields<T>();
        }
#endif // GLARE_BOOST_PFR_ENABLED
        else
        {
            static_assert(std::integral_constant<T, false>::value, "Reflection definition missing for type `T`.");
        }
    }

    // Aliases the default configuration of `reflect` to the `reflect_all` free-function.
    extern template void reflect<void>();
}