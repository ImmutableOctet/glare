#pragma once

#include "hash.hpp"

#include <util/reflection.hpp>

#include <string_view>
#include <string>
#include <utility>
#include <tuple>
#include <array>

namespace engine
{
    constexpr std::array short_name_prefixes =
    {
        "struct engine::",
         "class engine::",
          "enum engine::",
         "union engine::"
    };

	// Shortens the name of `T` if `T` belongs to the `engine` namespace.
    // 
    // See also: `engine_meta_type`
	template <typename T>
    constexpr std::string_view short_name()
    {
        return std::apply
        (
            [](auto&&... prefixes)
            {
                return util::resolve_short_name<T>(std::forward<decltype(prefixes)>(prefixes)...);
            },

            short_name_prefixes
        );
    }

    constexpr std::string optional_name(const auto& type_name)
    {
        return util::format("optional<{}>", type_name);
    }

    template <typename T>
    constexpr std::string optional_short_name()
    {
        return optional_name(short_name<T>());
    }

    constexpr std::string history_component_name(const auto& type_name)
    {
        return util::format("HistoryComponent<{}>", type_name);
    }

    template <typename T>
    constexpr std::string history_component_short_name()
    {
        return history_component_name(short_name<T>());
    }

    constexpr std::string_view as_short_name(std::string_view name_view)
    {
        return std::apply
        (
            [&name_view](auto&&... names)
            {
                return util::as_short_name(name_view, std::forward<decltype(names)>(names)...);
            },

            short_name_prefixes
        );
    }

    // Computes the hash associated with a `short_name`.
    // Useful for identifying a name local to the `engine` namespace by its opaque hashed value.
    // See also: `short_name`, `engine_meta_type`
    template <typename T>
    constexpr auto short_name_hash()
    {
        //return entt::type_hash<T>();

        return hash(short_name<T>());
    }

    template <typename T>
    constexpr auto optional_short_name_hash()
    {
        return hash(optional_short_name<T>());
    }

    template <typename T>
    constexpr auto history_component_short_name_hash()
    {
        return hash(history_component_short_name<T>());
    }

    // Resolves the meta-type for `type_name` as if it were part of the `engine` namespace.
    // (i.e. `type_name` should be a namespace-local identifier)
    inline entt::meta_type meta_type_from_name(std::string_view type_name)
    {
        return util::meta_type_from_name(type_name, "engine");
    }
}