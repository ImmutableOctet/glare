#pragma once

#include "types.hpp"
#include "hash.hpp"

#include <string>
#include <string_view>

#include <cstdint>

namespace engine
{
	// Returns true if `type` is considered a primitive type.
    // 
    // The `strings_as_primitives` parameter controls whether `std::string`
    // and `std::string_view` are considered primitive as well.
    bool type_is_primitive(const MetaType& type, bool strings_as_primitives=true);

    // Returns true if the type referenced by `type_id` is considered a primitive type.
    bool type_is_primitive(MetaTypeID type_id, bool strings_as_primitives=true);

    // Returns true if the `value` specified is a primitive type.
    // 
    // The `strings_as_primitives` parameter controls whether `std::string`
    // and `std::string_view` are considered primitive as well.
    bool value_is_primitive(const MetaAny& value, bool strings_as_primitives=true);

	// Executes the generic call operator of `callback` using the primitive `type` specified.
	// If `type` does not represent a primitive, this will return false and `callback` will not be executed.
	// 
	// The `strings_as_primitives` parameter controls whether `std::string`
	// and `std::string_view` are considered primitive as well.
    template <typename GenericLambdaCallback>
	bool observe_primitive_type(const MetaType& type, GenericLambdaCallback&& callback, bool strings_as_primitives=true)
	{
		using namespace engine::literals;

		if (!type)
		{
			return false;
		}

		auto notify = [&callback]<typename T>()
		{
			callback.operator()<T>();

			return true;
		};

		const auto type_id = type.id();

		switch (type_id)
		{
			case entt::type_hash<bool>::value():
				return notify.operator()<bool>();

			case entt::type_hash<std::int64_t>::value():
				return notify.operator()<std::int64_t>();

			case entt::type_hash<std::int32_t>::value():
				return notify.operator()<std::int32_t>();

			case entt::type_hash<std::int16_t>::value():
				return notify.operator()<std::int16_t>();

			case entt::type_hash<std::int8_t>::value():
				return notify.operator()<std::int8_t>();

			case entt::type_hash<std::uint64_t>::value():
				return notify.operator()<std::uint64_t>();

			case entt::type_hash<std::uint32_t>::value():
				return notify.operator()<std::uint32_t>();
			
			case entt::type_hash<std::uint16_t>::value():
				return notify.operator()<std::uint16_t>();
			
			case entt::type_hash<std::uint8_t>::value():
				return notify.operator()<std::uint8_t>();
			
			case entt::type_hash<long double>::value():
				return notify.operator()<long double>();
			
			case entt::type_hash<double>::value():
				return notify.operator()<double>();
			
			case entt::type_hash<float>::value():
				return notify.operator()<float>();
		}

		if (strings_as_primitives)
		{
			switch (type_id)
			{
				case "string"_hs: // entt::type_hash<std::string>::value():
					return notify.operator()<std::string>();

				case "string_view"_hs: // entt::type_hash<std::string_view>::value():
					return notify.operator()<std::string_view>();
			}
		}

		return false;
	}

	// Executes the generic call operator of `callback` using the primitive type referenced by `type_id`.
	// If `type_id` does not represent a primitive, this will return false and `callback` will not be executed.
	// 
	// The `strings_as_primitives` parameter controls whether `std::string`
	// and `std::string_view` are considered primitive as well.
	template <typename GenericLambdaCallback>
	bool observe_primitive_type(MetaTypeID type_id, GenericLambdaCallback&& callback, bool strings_as_primitives=true)
	{
		return observe_primitive_type(resolve(type_id), std::forward<GenericLambdaCallback>(callback), strings_as_primitives);
	}

	// Returns true if the `value` specified has a reflected indirection function.
    bool value_has_indirection(const MetaAny& value, bool bypass_indirect_meta_any=false);

    // Returns true if the `type` specified has a reflected indirection function.
    bool type_has_indirection(const MetaType& type);

    // Returns true if the the type referenced by `type_id` has a reflected indirection function.
    bool type_has_indirection(MetaTypeID type_id);

    // Returns true if the `type` specified is a 'system' type.
    bool type_is_system(const MetaType& type);

    // Returns true if the type referenced by the `type_id` specified is a 'system' type.
    bool type_is_system(const MetaTypeID type_id);

    // Returns true if the `value` specified references a 'system'.
    bool value_is_system(const MetaAny& value);

	// Returns true if the `type` specified is a 'service' type.
	bool type_is_service(const MetaType& type);

	// Returns true if the type referenced by the `type_id` specified is a 'service' type.
	bool type_is_service(const MetaTypeID type_id);

	// Returns true if the `value` specified references a 'service'.
	bool value_is_service(const MetaAny& value);

    // Returns true if the `type` specified has the `global namespace` property.
    bool type_has_global_namespace_flag(const MetaType& type);

    // Returns true if the type identified by `type_id` has the `global namespace` property.
    bool type_has_global_namespace_flag(MetaTypeID type_id);

    // Returns true if the `type` specified has the `component` property.
    bool type_is_component(const MetaType& type);

    // Returns true if the type identified by `type_id` has the `component` property.
    bool type_is_component(MetaTypeID type_id);

    // Returns true if the `type` specified has the `history_component` property.
    bool type_is_history_component(const MetaType& type);

    // Returns true if the type identified by `type_id` has the `history_component` property.
    bool type_is_history_component(MetaTypeID type_id);

	// Returns true if the type specified has the `coroutine` property.
	bool type_is_coroutine(const MetaType& type);

	// Returns true if the type identified by `type_id` has the `coroutine` property.
	bool type_is_coroutine(MetaTypeID type_id);

	// Returns true if the `value` specified references a 'coroutine'.
	bool value_is_coroutine(const MetaAny& value);
}