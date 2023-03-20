#pragma once

#include <utility>

namespace util
{
	template <typename InputType, typename ValueType=InputType>
    ValueType min(InputType&& value)
    {
        return static_cast<ValueType>(value);
    }

    template <typename LeftType, typename RightType, typename ValueType=LeftType>
    ValueType min(LeftType&& left, RightType&& right)
    {
        return (left <= right)
            ? static_cast<ValueType>(left)
            : static_cast<ValueType>(right)
        ;
    }

    template <typename LeftType, typename RightType, typename ...Values>
    auto min(LeftType&& left, RightType&& right, Values&&... values)
    {
        return min
        (
            min
            (
                std::forward<LeftType>(left),
                std::forward<RightType>(right)
            ),

            std::forward<Values>(values)...
        );
    }

    template <typename InputType, typename ValueType=InputType>
    ValueType max(InputType&& value)
    {
        return static_cast<ValueType>(value);
    }

    template <typename LeftType, typename RightType, typename ValueType=LeftType>
    ValueType max(LeftType&& left, RightType&& right)
    {
        return (left >= right)
            ? static_cast<ValueType>(left)
            : static_cast<ValueType>(right)
        ;
    }

    template <typename LeftType, typename RightType, typename ...Values>
    auto max(LeftType&& left, RightType&& right, Values&&... values)
    {
        return max
        (
            max
            (
                std::forward<LeftType>(left),
                std::forward<RightType>(right)
            ),

            std::forward<Values>(values)...
        );
    }
}