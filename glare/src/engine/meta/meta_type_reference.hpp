#pragma once

#include "types.hpp"

namespace engine
{
	struct MetaEvaluationContext;

	// This acts as a simple abstraction around type-erased access to a component instance.
	struct MetaTypeReference
	{
		public:
			static MetaAny get_instance(MetaTypeID type_id, Registry& registry, Entity entity);

			static MetaAny set_instance(MetaAny&& instance, Registry& registry, Entity entity);
			static MetaAny set_instance(const MetaType& component_type, MetaAny&& instance, Registry& registry, Entity entity);

			// The identifier for the type of the component.
			MetaTypeID type_id;

			/*
				If enabled, the type of an assignment value must match `type_id`.
				
				If disabled, component construction/assignment will be performed as-is, potentially failing at runtime.
				
				See also: `emplace_meta_component`
			*/
			bool validate_assignment_type : 1 = true;

			// Attempts to default construct the type referenced by `type_id`. (Currently disabled)
			//MetaAny get() const;

			// Returns a reference to `instance` if `instance` has the same type identified by `type_id`.
			MetaAny get(const MetaAny& instance) const;

			// Retrieves an opaque reference to a component attached to `entity` with the type identified by `type_id`.
			MetaAny get(Registry& registry, Entity entity) const;

			// Attempts to retrieve an opaque reference to a component attached to `entity` with the type identified by `type_id`.
			// If this fails, this function will attempt to retrieve a value from `context` instead.
			MetaAny get(Registry& registry, Entity entity, const MetaEvaluationContext& context) const;

			// Retrieves an opaque reference to a component attached to `target` with the type identified by `type_id`.
			MetaAny get(Entity target, Registry& registry, Entity context_entity) const;

			// Retrieves an opaque reference to a component attached to `target` with the type identified by `type_id`.
			MetaAny get(Entity target, Registry& registry, Entity context_entity, const MetaEvaluationContext& context) const;

			// Attempts to resolve `instance` using the context parameters provided.
			// If `instance` is the desired component-type (as identified by `type_id`), this will return a reference to `instance`.
			// If `instance` is an `Entity` value, this will attempt to retrieve an attached component (identified by `type_id`) from that entity.
			MetaAny get(const MetaAny& instance, Registry& registry, Entity context_entity) const;

			// Attempts to resolve `instance` using the context parameters provided.
			// If `instance` is the desired component-type (as identified by `type_id`), this will return a reference to `instance`.
			// If `instance` is an `Entity` value, this will attempt to retrieve an attached component (identified by `type_id`) from that entity.
			MetaAny get(const MetaAny& instance, Registry& registry, Entity context_entity, const MetaEvaluationContext& context) const;

			// Attempts to retrieve a value from `context` matching the type referenced by `type_id`.
			// (Useful for retrieving references to 'system' types)
			MetaAny get(const MetaEvaluationContext& context) const;

			// Attempts to assign `destination` to `source`, given that `source` and `destination`
			// exist, and that `destination` has the type identified by `type_id`.
			MetaAny set(MetaAny& source, MetaAny& destination);

			// Attempts to emplace/replace the component attached to `entity` (referenced by `type_id`) with `instance`.
			// This will fail if `instance` is not the same type as what is specified by `type_id`.
			MetaAny set(MetaAny& instance, Registry& registry, Entity entity);

			// Attempts to emplace/replace the component attached to `entity` (referenced by `type_id`) with `instance`.
			// This will fail if `instance` is not the same type as what is specified by `type_id`.
			MetaAny set(MetaAny& instance, Registry& registry, Entity entity, const MetaEvaluationContext& context);

			// Attempts to set `destination`'s instance of the underlying type (identified by `type_id`) to `source`'s instance.
			// If either `source` or `destination` is an entity type, this will attempt to reference that entity's component instance in place of the entity.
			// NOTE: If an entity-to-component resolution takes place for the `source` argument, a copy is produced to avoid modification/moved-from side effects.
			MetaAny set(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity);

			// Attempts to set `destination`'s instance of the underlying type (identified by `type_id`) to `source`'s instance.
			// If either `source` or `destination` is an entity type, this will attempt to reference that entity's component instance in place of the entity.
			// NOTE: If an entity-to-component resolution takes place for the `source` argument, a copy is produced to avoid modification/moved-from side effects.
			MetaAny set(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity, const MetaEvaluationContext& context);

			bool has_type() const;
			MetaType get_type() const;

			bool is_system_type() const;
		private:
			// NOTE: This function is non-const to avoid transitive const behavior.
			template <typename SelfType, typename InstanceType, typename ...Args>
			static MetaAny get_from_impl(SelfType&& self, InstanceType&& instance, Args&&... args);

			template <typename ...Args>
			MetaAny get_from_entity_impl(Entity entity, Registry& registry, Args&&... args) const;

			// This overload exists to catch any permutation without a `Registry` parameter.
			template <typename ...Args>
			MetaAny get_from_entity_impl(Entity entity, Args&&... args) const
			{
				return {};
			}
	};

	using MetaComponentReference = MetaTypeReference;
}