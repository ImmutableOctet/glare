#pragma once

#include <memory>
#include <tuple>

#include <math/math.hpp>

//#define FMT_HEADER_ONLY

#include <spdlog/spdlog.h>
#include <fmt/core.h>

/*
namespace spdlog
{
    class logger;
}
*/

namespace util
{
	using Logger = std::shared_ptr<spdlog::logger>;

	namespace log
	{
        //Logger get_console();
        //Logger get_error_logger();

        extern Logger console;
        extern Logger err_logger;

        void init();

        template <typename ...Args>
        inline Logger print(Args&& ...args)
        {
            console->info(std::forward<Args>(args)...);

            return console;
        }

        template <typename ...Args>
        inline Logger print_warn(Args&& ...args)
        {
            console->warn(std::forward<Args>(args)...);

            return console;
        }
	}
}

using util::log::print;
using util::log::print_warn;

template <>
struct fmt::formatter<math::Vector>
{
    constexpr auto parse(format_parse_context& ctx)
    {
        auto it = ctx.begin(), end = ctx.end();

        // Check if reached the end of the range:
        if (it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const math::Vector& v, FormatContext& ctx) -> decltype(ctx.out())
    {
        return format_to
		(
            ctx.out(),
            fmt::format_string<const char*>("({:.1f}, {:.1f}, {:.1f})"), // basic_format_string
            v.x, v.y, v.z
		);
    }
};

template <>
struct fmt::formatter<math::TransformVectors>
{
    constexpr auto parse(format_parse_context& ctx)
    {
        auto it = ctx.begin(), end = ctx.end();

        // Check if reached the end of the range:
        if (it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const math::TransformVectors& v, FormatContext& ctx)
    {
        const auto& [position, rotation, scale] = v;

        return format_to
        (
            ctx.out(),
            "[({:.1f}, {:.1f}, {:.1f}), ({:.1f}, {:.1f}, {:.1f}), ({:.1f}, {:.1f}, {:.1f})]",
            position.x, position.y, position.z,
            rotation.x, rotation.y, rotation.z,
            scale.x, scale.y, scale.z
        );
    }
};