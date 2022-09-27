#pragma once

#include "types.hpp"

// TODO: Move to separate source file.
#include <math/math.hpp>
#include <math/fixed_range.hpp>

#include <cmath>
#include <limits>
#include <optional>

namespace app::input
{
    struct GamepadDeadZone
    {
        using UnsignedType = std::uint16_t;
        using IntegralType = std::int16_t;
        using FloatingType = float;

        using Range = math::FixedRange<IntegralType, FloatingType>;

        static constexpr UnsignedType AXIS_RANGE       = std::numeric_limits<UnsignedType>::max(); // 65535
        static constexpr IntegralType MIN_AXIS_VALUE   = Range::absolute_min();
        static constexpr FloatingType MIN_AXIS_VALUE_F = Range::absolute_min_f();
        static constexpr IntegralType MAX_AXIS_VALUE   = Range::absolute_max();
        static constexpr FloatingType MAX_AXIS_VALUE_F = Range::absolute_max_f();

        static constexpr FloatingType MIN_AXIS_VALUE_NORMALIZED = -1.0f;
        static constexpr FloatingType MAX_AXIS_VALUE_NORMALIZED =  1.0f;

        struct Analog
        {
            std::optional<Range> x = std::nullopt;
            std::optional<Range> y = std::nullopt;

            inline FloatingType get(const std::optional<Range>& range, auto raw_value) const
            {
                if (range)
                {
                    return range->get(raw_value);
                }

                return Range::to_floating_point(raw_value);
            }

            inline auto get_x(auto raw_value) const
            {
                return get(x, raw_value);
            }

            inline auto get_y(auto raw_value) const
            {
                return get(y, raw_value);
            }
        };

        Analog left_analog;
        Analog right_analog;
        Analog triggers;

        inline GamepadDeadZone(const Analog& left_analog={}, const Analog& right_analog={}, const Analog& triggers={}) :
            left_analog(left_analog), right_analog(right_analog), triggers(triggers) {}
    };
}