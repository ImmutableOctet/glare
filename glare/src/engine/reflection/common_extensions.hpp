#pragma once

#include <engine/types.hpp>
#include <engine/meta/types.hpp>

#include <type_traits>
//#include <cmath>

namespace engine::impl
{
	template <typename T>
    bool operator_bool_impl(const T& value)
    {
        //return static_cast<bool>(value);

        if constexpr (std::is_floating_point_v<std::decay_t<T>>)
        {
            if (value > static_cast<T>(0.0))
            {
                return true;
            }
        }
        else
        {
            if (value)
            {
                return true;
            }
        }

        return false;
    }

    template <typename T>
    bool operator_logical_not_impl(const T& value)
    {
        return !operator_bool_impl<T>(value);
    }

    // Implements the inequality operator using the equality operator.
    template <typename TypeA, typename TypeB>
    bool inequality_operator_fallback_impl(const TypeA& a, const TypeB& b)
    {
        return !(a == b);
    }

    // Implements the equality operator using the inequality operator.
    template <typename TypeA, typename TypeB>
    bool equality_operator_fallback_impl(const TypeA& a, const TypeB& b)
    {
        return !(a != b);
    }

    template <typename T, typename OutputType=T>
    OutputType operator_unary_minus_impl(const T& value)
    {
        return static_cast<OutputType>(-value);
    }

    template <typename T, typename OutputType=T>
    OutputType operator_unary_plus_impl(const T& value)
    {
        if constexpr (std::is_arithmetic_v<std::decay_t<T>>)
        {
            if (value < static_cast<T>(0))
            {
                return operator_unary_minus_impl<T, OutputType>(value);
            }

            //return static_cast<OutputType>(std::abs(value));
        }

        return static_cast<OutputType>(value);
    }

    // Retrieves a pointer to `T` from `value`.
    // 
    // If `value` holds a `T` instance, this will return a pointer as expected.
    // If `value` is empty, or if the value is of a different type, this will return `nullptr`.
    template <typename T>
    T* from_meta(MetaAny& value)
    {
        if (!value)
        {
            return nullptr;
        }

        /*
        if
        (
            (value.type().id() == short_name_hash<T>().value())
            ||
            (value.type().id() == entt::type_hash<T>::value())
        )
        {
            return reinterpret_cast<T*>(value.data());
        }

        return nullptr;
        */

        return value.try_cast<T>();
    }
}