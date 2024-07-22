#pragma once

#include "types.hpp"
#include "meta_any.hpp"
#include "resolve.hpp"
#include "hash.hpp"

#include <entt/meta/meta.hpp>

#include <type_traits>
#include <utility>

namespace engine
{
    inline bool meta_type_is_container_wrapper(const MetaType& container_wrapper_type)
    {
        using namespace engine::literals;

        if (!container_wrapper_type)
        {
            return false;
        }

        return static_cast<bool>(container_wrapper_type.prop("container wrapper"_hs));
    }

    inline bool meta_type_is_container(const MetaType& container_type, bool allow_wrappers=false)
    {
        if (!container_type)
        {
            return false;
        }

        return
        (
            (container_type.is_sequence_container() || container_type.is_associative_container())
            ||
            (
                (allow_wrappers)
                &&
                (meta_type_is_container_wrapper(container_type))
            )
        );
    }

    inline bool meta_any_is_container(const MetaAny& container, bool allow_wrappers=false)
    {
        if (!container)
        {
            return false;
        }

        const auto container_type = container.type();

        if (!container_type)
        {
            return false;
        }

        return meta_type_is_container(container_type, allow_wrappers);
    }


    inline MetaTypeID try_get_wrapped_container_type_id(const MetaType& container_wrapper_type)
    {
        using namespace engine::literals;

        if (!meta_type_is_container_wrapper(container_wrapper_type))
        {
            return {};
        }

        if (const auto get_fn = container_wrapper_type.func("get_wrapped_container_type_id"_hs))
        {
            if (const auto result = get_fn.invoke({})) // "get_wrapped_container_type"_hs
            {
                if (const auto as_container_type_id = result.try_cast<MetaTypeID>())
                {
                    return *as_container_type_id;
                }
            }
        }

        return {};
    }

    inline MetaType try_get_wrapped_container_type(const MetaType& container_wrapper_type)
    {
        /*
        // Alternative implementation; unused due to potential for underlying type to be unreflected.
        if (const auto type_id = try_get_wrapped_container_type_id(container_wrapper_type))
        {
            return resolve(type_id);
        }
        */

        using namespace engine::literals;

        if (!meta_type_is_container_wrapper(container_wrapper_type))
        {
            return {};
        }

        if (const auto get_fn = container_wrapper_type.func("get_wrapped_container_type"_hs))
        {
            if (const auto result = get_fn.invoke({})) // "get_wrapped_container_type"_hs
            {
                if (const auto as_container_type = result.try_cast<MetaType>())
                {
                    return *as_container_type;
                }
            }
        }

        return {};
    }

    inline MetaTypeID try_get_container_value_type_id(const MetaType& container_type)
    {
        using namespace engine::literals;

        if (!meta_type_is_container(container_type, true))
        {
            return {};
        }

        if (const auto get_fn = container_type.func("get_value_type_id"_hs))
        {
            if (const auto result = get_fn.invoke({})) // "get_value_type"_hs
            {
                if (const auto as_value_type_id = result.try_cast<MetaTypeID>())
                {
                    return *as_value_type_id;
                }
            }
        }

        return {};
    }

    inline MetaType try_get_container_value_type(const MetaType& container_type)
    {
        if (const auto type_id = try_get_container_value_type_id(container_type))
        {
            return resolve(type_id);
        }

        return {};
    }

    inline MetaTypeID try_get_container_key_type_id(const MetaType& container_type)
    {
        using namespace engine::literals;

        if (!meta_type_is_container(container_type, true))
        {
            return {};
        }

        /*
        // Disabled for now; custom types (e.g. container wrappers) may utilize this API.
        if (!container_type.is_associative_container())
        {
            return resolve<std::size_t>().id(); // std::uint32_t
        }
        */

        if (const auto get_fn = container_type.func("get_key_type_id"_hs))
        {
            if (const auto result = get_fn.invoke({})) // "get_key_type"_hs
            {
                if (const auto as_key_type_id = result.try_cast<MetaTypeID>())
                {
                    return *as_key_type_id;
                }
            }
        }

        return {};
    }

    inline MetaType try_get_container_key_type(const MetaType& container_type)
    {
        if (!meta_type_is_container(container_type, true))
        {
            return {};
        }

        /*
        // Disabled for now; custom types (e.g. container wrappers) may utilize this API.
        if (!container_type.is_associative_container())
        {
            return resolve<std::size_t>();
        }
        */

        if (const auto type_id = try_get_container_key_type_id(container_type))
        {
            return resolve(type_id);
        }

        return {};
    }

    inline MetaTypeID try_get_container_pair_type_id(const MetaType& container_type)
    {
        using namespace engine::literals;

        if (!container_type)
        {
            return {};
        }

        /*
        // Disabled for now; custom types (e.g. container wrappers) may utilize this API.
        if (!container_type.is_associative_container())
        {
            return {};
        }
        */

        if (const auto get_fn = container_type.func("get_pair_type_id"_hs))
        {
            if (const auto result = get_fn.invoke({})) // "get_pair_type"_hs
            {
                if (const auto as_pair_type_id = result.try_cast<MetaTypeID>())
                {
                    return *as_pair_type_id;
                }
            }
        }

        return {};
    }

    inline MetaType try_get_container_pair_type(const MetaType& container_type)
    {
        if (const auto type_id = try_get_container_pair_type_id(container_type))
        {
            return resolve(type_id);
        }

        return {};
    }

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