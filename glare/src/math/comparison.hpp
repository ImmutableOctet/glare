#pragma once

#include <cmath>
#include <limits>

namespace math
{
    // NOTE: This may change in the future.
    template <typename FloatType>
    inline constexpr auto _EPSILON = std::numeric_limits<FloatType>::epsilon();

    // Floating-point epsilon value; used primarily for comparison.
    inline constexpr auto EPSILON = _EPSILON<float>;

    // Returns true if `left` and `right` are within `epsilon` distance from eachother.
    template <typename FloatType>
    bool float_equal(FloatType left, FloatType right, FloatType epsilon=_EPSILON<FloatType>) // constexpr
    {
        return (std::fabs(left - right) < epsilon); // std::abs
    }

    template <typename FloatType>
    bool float_unequal(FloatType left, FloatType right, FloatType epsilon=_EPSILON<FloatType>) // constexpr
    {
        return !float_equal(left, right, epsilon);
    }

    // Shorthand:
    template <typename FloatType>
    FloatType fcmp(FloatType left, FloatType right, FloatType epsilon=_EPSILON<FloatType>)
    {
        return float_equal(left, right, epsilon);
    }
}