#pragma once

#include <string>
#include <string_view>

#include <type_traits>

namespace engine::impl
{
    template <typename LeftStringType, typename RightStringType=LeftStringType, typename StringTypeOut=std::string>
    StringTypeOut add_strings(const LeftStringType& left, const RightStringType& right)
    {
        if constexpr (std::is_same_v<std::decay_t<LeftStringType>, std::decay_t<StringTypeOut>>)
        {
            if constexpr (std::is_same_v<std::decay_t<RightStringType>, std::decay_t<StringTypeOut>>)
            {
                return static_cast<StringTypeOut>(left + right);
            }
            else
            {
                return (left + static_cast<StringTypeOut>(right));
            }
        }
        else
        {
            if constexpr (std::is_same_v<std::decay_t<RightStringType>, std::decay_t<StringTypeOut>>)
            {
                return (static_cast<StringTypeOut>(left) + right);
            }
            else
            {
                return (static_cast<StringTypeOut>(left) + static_cast<StringTypeOut>(right));
            }
        }
    }

    template <typename LeftStringType, typename RightStringType=LeftStringType>
    bool string_equality(const LeftStringType& left, const RightStringType& right)
    {
        return (left == right);
    }

    template <typename LeftStringType, typename RightStringType=LeftStringType>
    bool string_inequality(const LeftStringType& left, const RightStringType& right)
    {
        return (left != right);
    }

    template <typename StringType>
    bool string_bool_impl(const StringType& str)
    {
        return !str.empty();
    }

    template <typename StringType, typename PrefixStringType=StringType>
    bool string_starts_with_impl(const StringType& str, const PrefixStringType& prefix)
    {
        return str.starts_with(prefix);
    }

    template <typename StringType, typename PrefixStringType=StringType>
    bool string_ends_with_impl(const StringType& str, const PrefixStringType& suffix)
    {
        return str.ends_with(suffix);
    }

    template <typename StringType, typename ValueType=StringType>
    bool string_contains_impl(const StringType& str, const ValueType& value)
    {
        return str.contains(value);
    }
}