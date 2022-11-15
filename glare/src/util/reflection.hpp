#pragma once

//#include <entt/meta/meta.hpp>
#include <entt/entt.hpp>

#include <string>
#include <string_view>
//#include <type_traits>

#include <regex>

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
        std::string_view name_view = entt::type_name<T>::value();

        const auto name_decl_rgx = std::regex("(struct|class|enum|union) ([\\w]+\\:\\:)(\\w+)");
        constexpr std::size_t type_name_index = 3;

        std::smatch rgx_match;

        if (!std::regex_search(name_view.begin(), name_view.end(), rgx_match, name_decl_rgx))
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

        auto try_processed = [&type_name, &namespace_symbol](const auto type_symbol) -> entt::meta_type
        {
            const auto processed_string = format("{} {}::{}", type_symbol, namespace_symbol, type_name);
            const auto hashed = entt::hashed_string(processed_string.data(), processed_string.size());

            if (auto processed = entt::resolve(hashed))
            {
                return processed;
            }

            return {};
        };

        if (auto struct_test = try_processed("struct"))
        {
            return struct_test;
        }

        if (auto class_test = try_processed("class"))
        {
            return class_test;
        }

        if (auto enum_test = try_processed("enum"))
        {
            return enum_test;
        }

        if (auto union_test = try_processed("union"))
        {
            return union_test;
        }

        return {};
    }

    // Helper function for shortening the generated type-name string from `entt`.
    // 
    // TODO: Make this into a variadic template.
	template <typename T>
    constexpr std::string_view resolve_short_name
    (
        auto struct_prefix,
        auto class_prefix,
        auto enum_prefix,
        auto union_prefix
    )
    {
        std::string_view name_view = entt::type_name<T>::value();

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

        if (clean_name(struct_prefix))
        {
            return name_view;
        }
        
        if (clean_name(class_prefix))
        {
            return name_view;
        }

        if (clean_name(enum_prefix))
        {
            return name_view;
        }

        if (clean_name(union_prefix))
        {
            return name_view;
        }

        return name_view;
    }
}