#pragma once

// General-purpose utilities for working with reflection.

#include "format.hpp"
#include "small_vector.hpp"

//#include <entt/meta/meta.hpp>
#include <entt/entt.hpp>

#include <string>
#include <string_view>
//#include <type_traits>

#include <regex>
#include <array>
#include <tuple>
#include <utility>

namespace entt
{
    // NOTE: This template specialization uses an internal/partially-documented portion of EnTT's API.
    template<typename... Args>
    struct meta_sequence_container_traits<util::small_vector<Args...>>
        : internal::basic_meta_sequence_container_traits<util::small_vector<Args...>> {};
}

namespace util
{
    // Helper function for shortening the generated type-name string from `entt`.
    // 
    // NOTE: This overload assumes you know nothing about the underlying namespace path.
    // Because this is unknown, we need to process the type-name using regular expression.
    // 
    // TODO: Switch to a faster regular expression library.
    template <typename T>
    inline std::string_view resolve_short_name() // std::string
    {
        // NOTE: The value retrieved from entt should be NULL-terminated. (see below usage)
        auto name_view = entt::type_name<T>::value();

        const auto name_decl_rgx = std::regex("(struct|class|enum|union) ([\\w]+\\:\\:)(\\w+)");
        constexpr std::size_t type_name_index = 3;

        std::smatch rgx_match;

        // TODO: Look into `std::regex` limitations with `std::string_view`.
        if (!std::regex_search(name_view.data(), rgx_match, name_decl_rgx)) // name_view.begin(), name_view.end()
        {
            assert(false);

            return {};
        }

        return { (name_view.data() + rgx_match.position(type_name_index)), rgx_match.length(type_name_index) };
    }

    inline entt::meta_type meta_type_from_name(std::string_view type_name, std::string_view namespace_symbol)
    {
        if (auto raw_string = entt::resolve(entt::hashed_string(type_name.data(), type_name.size())))
        {
            return raw_string;
        }

        entt::meta_type type_out;

        auto try_processed = [&type_name, &namespace_symbol, &type_out](const auto type_symbol)
        {
            const auto processed_string = util::format("{} {}::{}", type_symbol, namespace_symbol, type_name);
            const auto hashed = entt::hashed_string(processed_string.data(), processed_string.size());

            if (auto processed = entt::resolve(hashed))
            {
                type_out = processed;

                return true;
            }

            return false;
        };

        static constexpr std::array type_symbols =
        {
            "struct",
            "class",
            "enum",
            "union"
        };

        if
        (
            std::apply
            (
                [&try_processed](auto&&... symbols)
                {
                    return (try_processed(symbols) || ...);
                },

                type_symbols
            )
        )
        {
            return type_out;
        }

        return {};
    }

    template <typename ...Prefix>
    constexpr std::string_view as_short_name(std::string_view name_view, Prefix&&... prefix)
    {
        auto clean_name = [&name_view](std::string_view prefix) -> bool
        {
            if (name_view.starts_with(prefix))
            {
                const auto offset = prefix.size();

                name_view = { (name_view.data() + offset), (name_view.size() - offset) };

                return true;
            }

            return false;
        };

        if ((clean_name(prefix) || ...))
        {
            return name_view;
        }

        return name_view;
    }

    // Helper function for shortening the generated type-name string from `entt`.
    // 
    // TODO: Make this into a variadic template.
	template <typename T, typename ...Prefix>
    constexpr std::string_view resolve_short_name(Prefix&&... prefix)
    {
        return as_short_name(entt::type_name<T>::value(), std::forward<Prefix>(prefix)...);
    }
}