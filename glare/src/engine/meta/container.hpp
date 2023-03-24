#pragma once

#include "types.hpp"

#include <entt/meta/meta.hpp>

#include <type_traits>
#include <utility>

namespace engine
{
	template <typename ValueType>
    MetaAny try_get_container_wrapper(ValueType&& opaque_value)
    {
        static_assert(std::is_same_v<std::decay_t<ValueType>, MetaAny>, "The `opaque_value` parameter must be a `MetaAny` object, or a reference to one.");

        if (!opaque_value)
        {
            return {};
        }

        const auto value_type = opaque_value.type();

        if (!value_type)
        {
            return {};
        }

        if (value_type.is_sequence_container())
        {
            if (auto as_sequence_container = opaque_value.as_sequence_container())
            {
                return as_sequence_container;
            }
        }
        else if (value_type.is_associative_container())
        {
            if (auto as_associative_container = opaque_value.as_associative_container())
            {
                return as_associative_container;
            }
        }

        return {};
    }

    // TODO: Add wrapping routine for underlying container type
    // in place of `ValueType`, rather than just `MetaAny`.
    template <typename ValueType>
    entt::meta_sequence_container try_get_sequence_container(ValueType&& opaque_value)
    {
        static_assert(std::is_same_v<std::decay_t<ValueType>, MetaAny>, "The `opaque_value` parameter must be a `MetaAny` object, or a reference to one.");

        const auto value_type = opaque_value.type();

        if (!value_type)
        {
            return {};
        }

        if (value_type.is_sequence_container())
        {
            return opaque_value.as_sequence_container();
        }

        return {};
    }

    // TODO: Add wrapping routine for underlying container type
    // in place of `ValueType`, rather than just `MetaAny`.
    template <typename ValueType>
    entt::meta_associative_container try_get_associative_container(ValueType&& opaque_value)
    {
        static_assert(std::is_same_v<std::decay_t<ValueType>, MetaAny>, "The `opaque_value` parameter must be a `MetaAny` object, or a reference to one.");

        const auto value_type = opaque_value.type();

        if (!value_type)
        {
            return {};
        }

        if (value_type.is_associative_container())
        {
            return opaque_value.as_associative_container();
        }

        return {};
    }
}