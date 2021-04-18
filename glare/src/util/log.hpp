#pragma once

#include <spdlog/spdlog.h>
#include <math/math.hpp>
#include <tuple>

namespace util
{
	using Logger = std::shared_ptr<spdlog::logger>;

	namespace log
	{
		inline Logger get_console() // auto
		{
			return spdlog::get("console");
		}
	}
}

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
    auto format(const math::Vector& v, FormatContext& ctx)
    {
        return format_to
		(
            ctx.out(),
            "({:.1f}, {:.1f}, {:.1f})",
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