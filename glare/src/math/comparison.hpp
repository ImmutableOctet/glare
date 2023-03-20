#pragma once

#include <cmath>
#include <limits>

namespace math
{
    // NOTE: This may change in the future.
    template <typename FloatType>
    inline constexpr auto EPSILON = std::numeric_limits<FloatType>::epsilon();

    // Returns true if `left` and `right` are within `epsilon` distance from eachother.
    template <typename FloatType>
    bool float_equal(FloatType left, FloatType right, FloatType epsilon=EPSILON<FloatType>) // constexpr
    {
        return (std::fabs(left - right) < epsilon); // std::abs
    }

    template <typename FloatType>
    bool float_unequal(FloatType left, FloatType right, FloatType epsilon=EPSILON<FloatType>) // constexpr
    {
        return !float_equal(left, right, epsilon);
    }

    // Shorthand:
    template <typename FloatType>
    FloatType fcmp(FloatType left, FloatType right, FloatType epsilon=EPSILON<FloatType>)
    {
        return float_equal(left, right, epsilon);
    }
}