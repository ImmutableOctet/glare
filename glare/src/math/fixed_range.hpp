#pragma once

#include <limits>

namespace math
{
    /*
        Defines a fixed range:
            Starting at `min` (Negative),
            Centering at `threshold` (Positive value, but imposed in each direction),
            Ending at `max`. (ositive)

        Values are stored in a fixed integral range and are able to be converted into a normalized floating-point range (-1.0 to 0.0).
        
        NOTES:
            * `MIN_RANGE_VALUE` starts at `std::numeric_limits<IntegralType>::min()+1` because `max` is
            normally 1 lower than the corresponding power of 2, due to accounting for 0's representation.
    */
	template
    <
        typename IntegralType, typename FloatingType,
        IntegralType MIN_RANGE_VALUE=(std::numeric_limits<IntegralType>::min()+1), // Offset by 1 for symmetry.
        IntegralType MAX_RANGE_VALUE=(std::numeric_limits<IntegralType>::max())
    >
    struct FixedRange
    {
        using int_type   = IntegralType;
        using float_type = FloatingType;

        static constexpr IntegralType absolute_min()   { return MIN_RANGE_VALUE; }
        static constexpr IntegralType absolute_max()   { return MAX_RANGE_VALUE; }
        static constexpr FloatingType absolute_min_f() { return static_cast<FloatingType>(absolute_min()); }
        static constexpr FloatingType absolute_max_f() { return static_cast<FloatingType>(absolute_max()); }

        static constexpr FloatingType normalized_min()    { return static_cast<FloatingType>(-1.0); }
        static constexpr FloatingType normalized_center() { return static_cast<FloatingType>(0.0);  }
        static constexpr FloatingType normalized_max()    { return static_cast<FloatingType>(1.0);  }

        static constexpr IntegralType zero   = static_cast<IntegralType>(0);
        static constexpr FloatingType zero_f = static_cast<FloatingType>(0);

        // Converts an integral value to floating-point by dividing by
        // the absolute minimum/maximum value allowed.
        static constexpr FloatingType to_floating_point(IntegralType raw_value)
        {
            if (raw_value < 0)
            {
                return (static_cast<FloatingType>(raw_value) / -absolute_min_f());
            }

            return (static_cast<FloatingType>(raw_value) / absolute_max_f());
        }

        // Converts a floating-point value to integral by multiplying by
        // the absolute minimum/maximum value allowed.
        static constexpr IntegralType to_integral(FloatingType value)
        {
            if (value < 0)
            {
                return static_cast<IntegralType>(value * -absolute_min_f());
            }

            return static_cast<IntegralType>(value * absolute_max_f());
        }

        // Useful to have these for generic programming:
        static constexpr FloatingType to_floating_point(FloatingType value) { return value; }
        static constexpr IntegralType to_integral(IntegralType raw_value) { return raw_value; }

        // The minimum engagement threshold from the center-point (0).
        // Any value below this threshold in either the positive or negative direction is considered 0.
        IntegralType threshold;

        // The minimum and maximum values of this fixed-range.
        IntegralType min, max;

        /*
            Constructs an exact fixed-range using the threshold, min and max values specified.
            
            NOTES:
                * `threshold` shall be a positive value or exactly zero.
                * `min` shall not be lower than `MIN_RANGE_VALUE`.
                * `max` shall not be higher than `MAX_RANGE_VALUE`.
        */
        inline FixedRange(IntegralType threshold, IntegralType min, IntegralType max)
            : threshold(threshold), min(min), max(max) {}

        // This calls the integral equivalent by using the `to_integral` command on each value.
        inline FixedRange(FloatingType threshold, FloatingType min, FloatingType max)
            : FixedRange(to_integral(threshold), to_integral(min), to_integral(max)) {}

        // Constructs a fixed-range using the absolute min/max values, offset by the threshold itself.
        inline explicit FixedRange(IntegralType threshold)
            : FixedRange(threshold, (absolute_min()+threshold), (absolute_max()-threshold))
        {}

        // This constructor calls the integral equivalent after converting
        // appropriately to the absolute min/max range defined for this type.
        // (`MIN_RANGE_VALUE` to `MAX_RANGE_VALUE`)
        inline explicit FixedRange(FloatingType threshold)
            : FixedRange(to_integral(threshold))
        {}

        // A default fixed-range is assumed to use the absolute min/max values with no threshold.
        inline FixedRange()
            : threshold(0), min(absolute_min()), max(absolute_max())
        {}

        // Clamps an integral value to this range. If `raw_value` is less than `threshold`
        // in either the negative or positive direction, this will return 0.
        // If `renormalize` is true, this will offset the returned value by the starting threshold.
        inline IntegralType clamp(IntegralType raw_value, bool renormalize=true) const
        {
            //auto sign = math::sign<IntegralType, IntegralType>(raw_value);
            //auto offset_value = (raw_value - (threshold * sign));
            //return math::clamp<IntegralType>(offset_value, min, max);

            if (raw_value > threshold)
            {
                if (renormalize)
                {
                    return std::min((raw_value - threshold), max);
                }

                return std::min(raw_value, max);
            }
            else if (raw_value < -threshold)
            {
                if (renormalize)
                {
                    return std::max((raw_value + threshold), min);
                }

                return std::max(raw_value, min);
            }

            return {};
        }

        // Clamps a floating-point value to this range. This calls the
        // integral implementation and converts back and forth accordingly.
        inline FloatingType clamp(FloatingType raw_value, bool renormalize=true) const
        {
            return to_floating_point(clamp(to_integral(raw_value), renormalize));
        }

        inline bool within_threshold(IntegralType raw_value) const
        {
            return (std::abs(raw_value) < threshold); // <=
        }

        inline bool within_threshold(FloatingType value) const
        {
            return within_threshold(to_integral(value));
        }

        inline bool outside_threshold(IntegralType raw_value) const
        {
            return !within_threshold(raw_value);
        }

        inline bool outside_threshold(FloatingType value) const
        {
            return !within_threshold(value);
        }

        inline bool within_range(IntegralType raw_value, bool check_threshold=true) const
        {
            if (check_threshold)
            {
                if (within_threshold(raw_value))
                {
                    return false;
                }
            }

            if (raw_value < 0)
            {
                return (raw_value >= min);
            }
            else if (raw_value > 0)
            {
                return (raw_value <= max);
            }

            // If we're exactly zero, the threshold must
            // also be zero in order to be within range.
            return (threshold == 0);
        }

        inline bool within_range(FloatingType value, bool check_threshold = true) const
        {
            return within_range(to_integral(value), check_threshold);
        }

        inline bool outside_range(IntegralType raw_value, bool check_threshold=true) const
        {
            return !within_range(raw_value, check_threshold);
        }

        inline bool outside_range(FloatingType value, bool check_threshold=true) const
        {
            return !within_range(value, check_threshold);
        }

        inline bool operator<(IntegralType raw_value)  const { return within_range(raw_value);  }
        inline bool operator<(FloatingType value)      const { return within_range(value);      }
        inline bool operator<=(IntegralType raw_value) const { return within_range(raw_value);  }
        inline bool operator<=(FloatingType value)     const { return within_range(value);      }
        inline bool operator>(IntegralType raw_value)  const { return outside_range(raw_value); }
        inline bool operator>(FloatingType value)      const { return outside_range(value);     }
        inline bool operator>=(IntegralType raw_value) const { return outside_range(raw_value); }
        inline bool operator>=(FloatingType value)     const { return outside_range(value);     }

        // Returns a normalized value from -1.0 to 1.0 according to the specifications of this range.
        // `raw_value` should be a normalized value based on the `MIN_RANGE_VALUE`
        // and `MAX_RANGE_VALUE` parameters given when defining this type.
        // See also: `absolute_min`, `absolute_max`, `to_floating_point`
        inline FloatingType get(FloatingType raw_value, bool renormalize=true) const
        {
            auto center_f = to_floating_point(threshold);

            if (raw_value > center_f) // && (raw_value > 0)
            {
                auto value = ((renormalize) ? (raw_value - center_f) : raw_value);
                auto max_f = to_floating_point(max);

                return std::min((value / max_f), normalized_max());
            }
            else if (raw_value < -center_f) // && (raw_value < 0)
            {
                auto value = ((renormalize) ? (raw_value + center_f) : raw_value);
                auto min_f = to_floating_point(min);

                return std::max(-(value / min_f), normalized_min());
            }

            return 0.0f;
        }

        // Returns a normalized value from -1.0 to 1.0 according to the specifications of this range.
        inline FloatingType get(IntegralType raw_value, bool renormalize=true) const
        {
            return get(to_floating_point(raw_value), renormalize);
        }

        // Retrieves a normalized value from `get`, then converts back into integral range.
        inline IntegralType get_integral(IntegralType raw_value, bool renormalize=true) const
        {
            return to_integral(get(to_floating_point(raw_value), renormalize));
        }

        // Retrieves a normalized value from `get`, then converts back into integral range.
        inline IntegralType get_integral(FloatingType raw_value, bool renormalize=true) const
        {
            return to_integral(get(raw_value, renormalize));
        }
    };
}