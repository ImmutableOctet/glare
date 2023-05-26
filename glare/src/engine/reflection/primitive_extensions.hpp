#pragma once

#include "string_conversion.hpp"

namespace engine::impl
{
    template <typename ToType, typename FromType>
    ToType static_cast_impl(const FromType& from_value)
    {
        return static_cast<ToType>(from_value);
    }
}