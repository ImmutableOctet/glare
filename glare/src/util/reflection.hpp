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
    /*
    template <typename T, std::size_t preallocated, typename... Args>
    struct meta_sequence_container_traits<util::small_vector<T, preallocated, Args...>> // <util::small_vector<Args...>>
        : internal::basic_meta_sequence_container_traits<util::small_vector<T, preallocated, Args...>> {};
    */

    // Based on `basic_meta_sequence_container_traits` from EnTT's source code.
    template <typename T, std::size_t preallocated, typename... Args>
    struct meta_sequence_container_traits<util::small_vector<T, preallocated, Args...>>
    {
        // Native type:
        using Type = util::small_vector<T, preallocated>;

        using value_type            = typename Type::value_type;
        using const_reference       = typename Type::const_reference;

        using native_iterator       = typename Type::iterator;
        using native_const_iterator = typename Type::const_iterator;

        // EnTT API:
        using iterator              = meta_sequence_container::iterator;
        using size_type             = std::size_t;

        [[nodiscard]] static size_type size(const entt::any& container) noexcept
        {
            return any_cast<const Type&>(container).size();
        }

        [[nodiscard]] static bool resize([[maybe_unused]] entt::any& container, [[maybe_unused]] size_type sz)
        {
            // NOTE: Added default-constructible check added due to conflict/limitation with Folly's small vector implementation.
            if constexpr (std::is_default_constructible_v<std::decay_t<value_type>>) // && entt::internal::is_dynamic_sequence_container<Type>::value
            {
                if (auto* const underlying = any_cast<Type>(&container))
                {
                    underlying->resize(sz);

                    return true;
                }
            }

            return false;
        }

        [[nodiscard]] static iterator iter(const entt::meta_ctx& ctx, entt::any& container, const bool as_end)
        {
            if (auto* const underlying = any_cast<Type>(&container))
            {
                return iterator
                {
                    ctx,

                    (as_end)
                        ? underlying->end()
                        : underlying->begin()
                };
            }

            const Type& as_const = any_cast<const Type&>(container);

            return iterator
            {
                ctx,

                (as_end)
                    ? as_const.end()
                    : as_const.begin()
            };
        }

        [[nodiscard]] static iterator insert_or_erase
        (
            [[maybe_unused]] const entt::meta_ctx& ctx,
            [[maybe_unused]] entt::any& container,
            [[maybe_unused]] const entt::any& handle,
            [[maybe_unused]] entt::meta_any& value
        )
        {
            if constexpr (true) // entt::internal::is_dynamic_sequence_container<Type>::value
            {
                if (auto* const underlying = any_cast<Type>(&container))
                {
                    auto it = native_const_iterator {};

                    if (auto non_const = any_cast<native_iterator>(&handle))
                    {
                        it = *non_const;
                    }
                    else
                    {
                        it = any_cast<const native_const_iterator&>(handle);
                    }

                    if (value)
                    {
                        const auto* element = value.try_cast<std::remove_reference_t<const_reference>>();

                        return iterator
                        {
                            ctx,
                            
                            underlying->insert
                            (
                                it,

                                (element)
                                    ? *element
                                    : value.cast<value_type>()
                            )
                        };
                    }
                    else
                    {
                        return iterator
                        {
                            ctx,

                            underlying->erase(it)
                        };
                    }
                }
            }

            return iterator {};
        }
    };
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