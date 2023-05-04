#pragma once

#include <util/string.hpp>
#include <util/format.hpp>

#include <utility>
#include <type_traits>
#include <string_view>
#include <string>

namespace engine::impl
{
    template <typename T>
    std::string format_string_conv(const T& value)
    {
        return util::format("{}", value);
    }

    template <typename ArithmeticType, typename=std::enable_if_t<std::is_arithmetic_v<ArithmeticType>>>
    std::string arithmetic_to_string_impl(ArithmeticType value) // const ArithmeticType&
    {
        if constexpr (std::is_same_v<std::decay_t<ArithmeticType>, bool>)
        {
            if (value)
            {
                return "true";
            }
            else
            {
                return "false";
            }
        }
        else
        {
            return std::to_string(value);
        }
    }

    template <typename T>
    T from_string_view_impl(std::string_view value) // const std::string_view&
    {
        // TODO: Determine if `bool` should have different behavior here.
        // (e.g. checking against `value.empty()` instead of looking for `true`-like values)

        if (auto result = util::from_string<T>(value, true))
        {
            return *result;
        }

        return {};
    }

    template <typename T>
    T from_string_impl(const std::string& value)
    {
        return from_string_view_impl<T>(value);
    }
}