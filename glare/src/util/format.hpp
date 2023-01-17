#pragma once

#include <string_view>

//#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include "magic_enum.hpp"
#include <magic_enum_format.hpp>

// Standard alternative:
//#include <format>
//using fmt = std;

// {fmt} equivalent to `std::formatter` definition found in `magic_enum_format`.
template <typename E>
struct fmt::formatter<E, std::enable_if_t<std::is_enum_v<E>&& magic_enum::customize::enum_format_enabled<E>(), char>> : fmt::formatter<std::string_view, char> {
    auto format(E e, format_context& ctx) {
        static_assert(std::is_same_v<char, string_view::value_type>, "formatter requires string_view::value_type type same as char.");
        using D = std::decay_t<E>;

        if constexpr (magic_enum::detail::supported<D>::value) {
            if (const auto name = magic_enum::enum_name<D, magic_enum::as_flags<magic_enum::detail::is_flags_v<D>>>(e); !name.empty()) {
                return fmt::formatter<std::string_view, char>::format(std::string_view{ name.data(), name.size() }, ctx);
            }
        }
        return fmt::formatter<std::string_view, char>::format(std::to_string(magic_enum::enum_integer<D>(e)), ctx);
    }
};

//namespace engine { using fmt::format; }

namespace util
{
	using fmt::format;
}