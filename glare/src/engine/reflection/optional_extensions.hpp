#pragma once

#include <engine/meta/types.hpp>

#include <optional>
#include <stdexcept>

namespace engine::impl
{
	template <typename T>
    MetaAny optional_value_or_impl(std::optional<T>& self, MetaAny& value) // const MetaAny&
    {
        if (self)
        {
            return entt::forward_as_meta(self.value());
        }

        return value.as_ref();
    }

    template <typename T>
    const T& optional_to_const_ref(const std::optional<T>& self)
    {
        if (!self)
        {
            //assert(false);
            throw std::runtime_error("Attempted to access empty optional.");
        }

        return self.value();
    }

    template <typename T>
    T& optional_to_ref(std::optional<T>& self)
    {
        if (!self)
        {
            //assert(false);
            throw std::runtime_error("Attempted to access empty optional.");
        }

        return self.value();
    }

    template <typename T>
    T optional_to_value(const std::optional<T>& self)
    {
        return optional_to_const_ref<T>(self);
    }

    template <typename T>
    MetaAny optional_resolve_impl(const std::optional<T>& self)
    {
        if (self)
        {
            //return MetaAny { self.value() };

            return entt::forward_as_meta(self.value()); // self.value();
        }

        return {};
    }

    template <typename T>
    MetaTypeID type_id_from_optional()
    {
        //return short_name_hash<T>();

        auto type = resolve<T>();

        if (!type)
        {
            return {};
        }

        return type.id();
    }

    template <typename T>
    MetaType type_from_optional()
    {
        //return resolve(type_id_from_optional<T>());

        return resolve<T>();
    }

    template <typename T>
    MetaType optional_get_type_impl(const std::optional<T>& self)
    {
        return type_from_optional<T>();
    }
}