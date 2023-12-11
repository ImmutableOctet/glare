#pragma once

#include <cmath>

namespace math
{
    namespace impl
    {
        // Computes the oscillation of `position` within `area`.
        template <typename FloatType>
        FloatType oscillate_impl(FloatType position, FloatType area)
        {
            return std::abs(std::fmod((position + area), (area * static_cast<FloatType>(2.0))) - area);
        }
    }
    
    // Oscillates `position` between `min_value` and `max_value`.
    template <typename FloatType>
    FloatType oscillate(FloatType position, FloatType min_value, FloatType max_value)
    {
        const auto area = (max_value - min_value); // std::abs(...)

        return (min_value + impl::oscillate_impl(position, area));
    }

    // Oscillates `position` between 0.0 and 1.0.
    template <typename FloatType>
    FloatType oscillate(FloatType position)
    {
        return oscillate
        (
            position,
            
            static_cast<FloatType>(0.0),
            static_cast<FloatType>(1.0)
        );
    }

    // Oscillates `position_normalized` between -1.0 and 1.0.
    template <typename FloatType>
    FloatType oscillate_normalized(FloatType position_normalized)
    {
        return
        (
            oscillate
            (
                (position_normalized + static_cast<FloatType>(1.0)),

                static_cast<FloatType>(0.0),
                static_cast<FloatType>(2.0)
            )
            -
            static_cast<FloatType>(1.0)
        );
    }
}