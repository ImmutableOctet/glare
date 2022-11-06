#pragma once

#include "types.hpp"
#include "gamepad_analog.hpp"

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
            //std::optional<Range> z = std::nullopt;

            bool invert_x : 1 = false;
            bool invert_y : 1 = false;
            //bool invert_z : 1 = false;

            inline FloatingType get(const std::optional<Range>& range, auto raw_value, bool invert_value=false) const
            {
                FloatingType output;

                if (range)
                {
                    output = range->get(raw_value);
                }
                else
                {
                    output = Range::to_floating_point(raw_value);
                }

                if (invert_value)
                {
                    return -output;
                }

                return output;
            }

            inline auto get_x(auto raw_value) const
            {
                return get(x, raw_value, invert_x);
            }

            inline auto get_y(auto raw_value) const
            {
                return get(y, raw_value, invert_y);
            }

            inline bool beyond_threshold(const std::optional<Range>& range, auto raw_value) const
            {
                if (range)
                {
                    return range->outside_threshold(raw_value);
                }

                return static_cast<bool>(raw_value); // > 0
            }

            inline bool x_beyond_threshold(auto raw_value) const
            {
                return beyond_threshold(x, raw_value);
            }

            inline bool y_beyond_threshold(auto raw_value) const
            {
                return beyond_threshold(y, raw_value);
            }

            inline bool beyond_threshold(const math::Vector2D& values) const
            {
                if (x_beyond_threshold(values.x))
                {
                    return true;
                }

                if (y_beyond_threshold(values.y))
                {
                    return true;
                }

                return false;
            }
        };

        Analog left_analog;
        Analog right_analog;
        Analog triggers;
        //Analog dpad;

        inline const Analog* get_analog(GamepadAnalog analog) const
        {
            using enum GamepadAnalog;

            switch (analog)
            {
                case Left:
                    return &left_analog;
                case Right:
                    return &right_analog;
                case Triggers:
                    return &triggers;
                //case DPad:
                //    return &dpad;
            }

            return nullptr;
        }

        // Non-const variant of `get_analog`.
        inline Analog* get_analog(GamepadAnalog analog)
        {
            // Call const version, then cast back to non-const.
            return const_cast<Analog*>(const_cast<const GamepadDeadZone*>(this)->get_analog(analog));
        }

        inline GamepadDeadZone(const Analog& left_analog={}, const Analog& right_analog={}, const Analog& triggers={}) :
            left_analog(left_analog), right_analog(right_analog), triggers(triggers) {}
    };
}