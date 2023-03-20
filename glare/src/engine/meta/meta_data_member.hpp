#pragma once

#include "types.hpp"

namespace engine
{
	struct MetaEvaluationContext;

	// This acts as a simple abstraction around type-erased access to a data-member of a component.
	struct MetaDataMember
	{
		// The identifier for the type this data member belongs to.
		MetaTypeID type_id;

		// A type-local data identifier used to retrieve a value from an object instance of `type_id`.
		MetaSymbolID data_member_id;

		// Retrieves an instance of the type referenced by `type_id` from `entity` using `registry`.
		MetaAny get_instance(Registry& registry, Entity entity) const;

		// Retrieves the data-member referenced by `data_member_id` from `instance`.
		// If `instance` does not have the same identifier as `type_id`, this will return an empty object.
		MetaAny get(const MetaAny& instance) const;

		// Retrieves the data-member referenced by `data_member_id`
		// from an instance of `type_id` attached to `entity` in `registry`.
		// 
		// If an instance of `type_id` is not currently attached to `entity`, this will return an empty object.
		MetaAny get(Registry& registry, Entity entity) const;

		// Retrieves the data-member referenced by `data_member_id`
		// from an instance of `type_id` attached to `target` in `registry`.
		// 
		// If an instance of `type_id` is not currently attached to `target`, this will return an empty object.
		MetaAny get(Entity target, Registry& registry, Entity context_entity) const;

		// Retrieves the data-member referenced by `data_member_id` from `instance`.
		MetaAny get(const MetaAny& instance, Registry& registry, Entity context_entity) const;

		// Retrieves the data-member referenced by `data_member_id` from `instance`.
		MetaAny get(const MetaAny& instance, Registry& registry, Entity context_entity, const MetaEvaluationContext& context) const;

		// Assigns the data-member of `source` (referenced by `data_member_id`) to the `destination` specified.
		MetaAny set(MetaAny& source, MetaAny& destination);

		// Assigns the data-member of `source` (referenced by `data_member_id`) to the `destination` specified.
		MetaAny set(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity, const MetaEvaluationContext& context);

		// Assigns the data-member of `source` (referenced by `data_member_id`) to the `destination` specified.
		MetaAny set(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity);

		// Retrieves an instance of the targeted type (referenced by `type_id`) from `entity` (as a component) using `registry`,
		// then assigns its data-member (referenced by `data_member_id`) to `value`.
		MetaAny set(MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context); // MetaDataMember&

		// Retrieves an instance of the targeted type (referenced by `type_id`) from `entity` (as a component) using `registry`,
		// then assigns its data-member (referenced by `data_member_id`) to `value`.
		MetaAny set(MetaAny& value, Registry& registry, Entity entity); // MetaDataMember&

		bool has_member() const;
		entt::meta_data get_data() const;

		bool has_type() const;
		MetaType get_type() const;

		bool has_member_type() const;
		MetaType get_member_type() const;

		bool operator==(const MetaDataMember&) const noexcept = default;
		bool operator!=(const MetaDataMember&) const noexcept = default;
	};
}