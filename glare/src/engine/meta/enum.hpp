#pragma once

#include "types.hpp"

#include <util/magic_enum.hpp>

#include <string_view>
#include <optional>
#include <stdexcept>

// TODO: Migrate macros for enum-based bitwise operators.

//using namespace magic_enum::...;

namespace engine
{
	template <typename EnumType>
    std::optional<EnumType> try_string_to_enum_value(std::string_view enum_short_str)
    {
        if (auto result = magic_enum::enum_cast<EnumType>(enum_short_str))
        {
            return *result;
        }

        return std::nullopt;
    }

    template <typename EnumType>
    EnumType string_to_enum_value(std::string_view enum_short_str)
    {
        auto result = magic_enum::enum_cast<EnumType>(enum_short_str);

        if (!result)
        {
            throw std::invalid_argument("Invalid enum value specified.");
        }

        return *result;
    }

    template <typename EnumType>
    std::string_view enum_value_to_string_view(EnumType enum_value)
    {
        return magic_enum::enum_name(enum_value);
    }

    template <typename EnumType>
    std::string enum_value_to_string(EnumType enum_value)
    {
        return std::string { enum_value_to_string_view(enum_value) };
    }

	// Retrieves the runtime (property-based) value associated to `id` in `meta_type_inst`.
    // If no value is associated, this will fail via assertion.
    template <typename EnumType>
    EnumType get_reflected_enum(const entt::meta_type& meta_type_inst, entt::hashed_string id)
    {
        auto meta_property = meta_type_inst.prop(id);
        return meta_property.value().cast<EnumType>();
    }

    // Computes the hash of `enum_value_name`, then retrieves the runtime value associated by `meta_type_inst`.
    // If no value is associated, this will fail via assertion.
    template <typename EnumType>
    EnumType get_reflected_enum(const entt::meta_type& meta_type_inst, std::string_view enum_value_name)
    {
        return get_reflected_enum<EnumType>(meta_type_inst, entt::hashed_string(enum_value_name.data(), enum_value_name.size()));
    }

    // Retrieves a reflected enum value by its name or hash of its name.
    // If no value is associated (see `reflect_enum` and `reflect`), this will fail via assertion.
    template <typename EnumType, typename IdentifierType>
    EnumType get_reflected_enum(const IdentifierType& enum_value_identifier)
    {
        return get_reflected_enum<EnumType>(entt::resolve<EnumType>(), enum_value_identifier);
    }
};