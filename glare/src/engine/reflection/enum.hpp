#pragma once

#include "optional.hpp"

#include <engine/types.hpp>
#include <engine/meta/enum.hpp>

namespace engine
{
	// TODO: Look into this implementation again.
    // (Probably not very efficient to use properties for this)
    template <typename EnumType, bool generate_optional_reflection=true>
    auto reflect_enum(MetaTypeID type_id, bool values_as_properties=false)
    {
        auto type = custom_meta_type<EnumType>(type_id)
            .func<&string_to_enum_value<EnumType>>("string_to_value"_hs)
            .ctor<&string_to_enum_value<EnumType>>()
            .conv<&enum_value_to_string<EnumType>>()
        ;

        if (values_as_properties)
        {
            magic_enum::enum_for_each<EnumType>
            (
                [](EnumType enum_value)
                {
                    const auto enum_name = magic_enum::enum_name<EnumType>(enum_value);
                    const auto enum_value_id = hash(enum_name);

                    entt::meta<EnumType>().prop(enum_value_id, enum_value);
                }
            );
        }

        if constexpr (generate_optional_reflection)
        {
            engine_optional_type<EnumType>(false);
        }

        return type;
    }

    // NOTE: This is called automatically via `reflect` when `T` is an enumeration type.
    template <typename EnumType>
    void reflect_enum(bool values_as_properties=false)
    {
        reflect_enum<EnumType>(short_name_hash<EnumType>(), values_as_properties);
    }
}