#pragma once

#include "types.hpp"

#include <util/format.hpp>

template <>
struct fmt::formatter<math::Vector2D>
{
    // Presentation format: 'f' - fixed, 'e' - exponential.
    char presentation = 'f';

    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin(), end = ctx.end();

        if (it != end && (*it == 'f' || *it == 'e'))
            presentation = *it++;

        // Check if reached the end of the range.
        if (it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range.
        return it;
    }

    template <typename FormatContext>
    auto format(const math::Vector2D& v, FormatContext& ctx) -> decltype(ctx.out())
    {
        return (presentation == 'f')
            ? fmt::format_to(ctx.out(), "({:.1f}, {:.1f})", v.x, v.y)
            : fmt::format_to(ctx.out(), "({:.1e}, {:.1e})", v.x, v.y)
        ;
    }
};

template <>
struct fmt::formatter<math::vec2i>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const math::vec2i& v, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({0}, {0})", v.x, v.y);
    }
};

template <>
struct fmt::formatter<math::Vector3D>
{
    // Presentation format: 'f' - fixed, 'e' - exponential.
    char presentation = 'f';

    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin(), end = ctx.end();

        if (it != end && (*it == 'f' || *it == 'e'))
            presentation = *it++;

        // Check if reached the end of the range.
        if (it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range.
        return it;
    }

    template <typename FormatContext>
    auto format(const math::Vector3D& v, FormatContext& ctx) -> decltype(ctx.out())
    {
        return (presentation == 'f')
            ? fmt::format_to(ctx.out(), "({:.1f}, {:.1f}, {:.1f})", v.x, v.y, v.z)
            : fmt::format_to(ctx.out(), "({:.1e}, {:.1e}, {:.1e})", v.x, v.y, v.z)
        ;
    }
};

template <>
struct fmt::formatter<math::Vector4D>
{
    // Presentation format: 'f' - fixed, 'e' - exponential.
    char presentation = 'f';

    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin(), end = ctx.end();

        if (it != end && (*it == 'f' || *it == 'e'))
            presentation = *it++;

        // Check if reached the end of the range.
        if (it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range.
        return it;
    }

    template <typename FormatContext>
    auto format(const math::Vector4D& v, FormatContext& ctx) -> decltype(ctx.out())
    {
        return (presentation == 'f')
            ? fmt::format_to(ctx.out(), "({:.1f}, {:.1f}, {:.1f}, {:.1f})", v.x, v.y, v.z, v.w)
            : fmt::format_to(ctx.out(), "({:.1e}, {:.1e}, {:.1e}, {:.1e})", v.x, v.y, v.z, v.w)
        ;
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

template <>
struct fmt::formatter<math::Quaternion>
{
    // Presentation format: 'f' - fixed, 'e' - exponential.
    char presentation = 'f';

    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin(), end = ctx.end();

        if (it != end && (*it == 'f' || *it == 'e'))
            presentation = *it++;

        // Check if reached the end of the range.
        if (it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range.
        return it;
    }

    template <typename FormatContext>
    auto format(const math::Quaternion& q, FormatContext& ctx) -> decltype(ctx.out())
    {
        return (presentation == 'f')
            ? fmt::format_to(ctx.out(), "({:.1f}, {:.1f}, {:.1f}, {:.1f})", q.w, q.x, q.y, q.z)
            : fmt::format_to(ctx.out(), "({:.1e}, {:.1e}, {:.1e}, {:.1e})", q.w, q.x, q.y, q.z)
        ;
    }
};