#pragma once

#include <entt/meta/meta.hpp>
//#include <entt/entt.hpp>

#include "traits.hpp"
#include "enum.hpp"
#include "types.hpp"

#include <util/reflection.hpp>

#include <string_view>
#include <type_traits>

namespace engine
{
    using entt::resolve;

	// Shortens the name of `T` if `T` belongs to the `engine` namespace.
    // 
    // See also: `engine_meta_type`
	template <typename T>
    constexpr std::string_view short_name()
    {
        return util::resolve_short_name<T>
        (
            "struct engine::",
             "class engine::",
              "enum engine::",
             "union engine::"
        );
    }

    // Computes a hash for the string specified.
    constexpr entt::hashed_string hash(std::string_view str)
    {
        return entt::hashed_string(str.data(), str.size()); // str.length()
    }

    // Computes the hash associated with a `short_name`.
    // Useful for identifying a name local to the `engine` namespace by its opaque hashed value.
    // See also: `short_name`, `engine_meta_type`
    template <typename T>
    constexpr auto short_name_hash()
    {
        //return entt::type_hash<T>();

        return hash(short_name<T>());
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

    // Resolves the meta-type for `type_name` as if it were part of the `engine` namespace.
    // (i.e. `type_name` should be a namespace-local identifier)
    inline entt::meta_type meta_type_from_name(std::string_view type_name)
    {
        return util::meta_type_from_name(type_name, "engine");
    }

	/*
		Generates reflection meta-data for the `engine` module.
		Reflected meta-data can be queried via `entt`'s meta-type system.
		
		NOTES:
			* Although this is declared here, the function definition can be found in the `reflection` submodule.
			(This avoids multiple template instantiations)

			* Calling the `engine::reflect` free-function with `T=void` or with no
			template-specification will also result in a call this function.

			* Calling this function multiple times on the same thread is safe.
			
			* Calling this function from more than one thread concurrently is considered unsafe.
			(TODO: Look into fixing this)

			* The `Game` type automatically calls this function during construction.
			
			* The implementation of this function dictates the order-of-operations for reflected types.
			It is recommended that you do not inspect reflected types whilst meta factories are still being constructed.

			* The `primitives` argument is used to control whether we reflect simple enumeration types, structs, etc.
			* The `dependencies` argument determines if other supporting modules (e.g. `math`) are reflected.
	*/
	void reflect_all(bool primitives=true, bool dependencies=true);
}