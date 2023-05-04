#pragma once

#include <engine/types.hpp>
#include <engine/meta/types.hpp>

namespace engine::impl
{
	template <typename T>
    bool operator_bool_impl(const T& value)
    {
        //return static_cast<bool>(value);

        ///*
        if (value)
        {
            return true;
        }

        return false;
        //*/
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