#pragma once

#include "types.hpp"

#include <util/format.hpp>
//#include <vector>

template <>
struct fmt::formatter<engine::Entity>
{
    using IntType = engine::EntityIDType;

    formatter<IntType> int_formatter;

    constexpr auto parse(format_parse_context& ctx)
    {
        //return int_formatter.parse(ctx);

        auto it = ctx.begin(), end = ctx.end();

        // Check if reached the end of the range:
        if (it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const engine::Entity& e, FormatContext& ctx)
    {
        if (e == engine::null)
        {
            return fmt::format_to
            (
                ctx.out(),
                "null",
                static_cast<IntType>(e)
            );
        }

        return int_formatter.format(static_cast<IntType>(e), ctx);
    }
};

/*
template <>
struct fmt::formatter<std::vector<engine::Entity>>
{
    using IntType = engine::EntityIDType;

    formatter<std::vector<IntType>> int_formatter;

    constexpr auto parse(format_parse_context& ctx)
    {
        return int_formatter.parse(ctx);
    }

    template <typename FormatContext>
    auto format(const std::vector<engine::Entity>& ev, FormatContext& ctx)
    {
        return int_formatter.format(*reinterpret_cast<std::vector<IntType>>(&ev), ctx);
    }
};
*/