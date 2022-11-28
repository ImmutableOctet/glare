#pragma once

#include "meta/types.hpp"
#include "meta/meta_description.hpp"

#include "entity_state_rule.hpp"

// TODO: Forward declare JSON type.
#include <util/json.hpp>

#include <util/small_vector.hpp>

#include <optional>
#include <string_view>
#include <unordered_map>
//#include <map>

namespace engine
{
	struct FrozenStateComponent;
	struct StateStorageComponent;

	class EntityState
	{
		public:
			using RuleCollection = EntityStateRuleCollection;

			// TODO: Review whether it makes sense to pair keys and values in a `small_vector` of tuples, rather than a map.
			using Rules = std::unordered_map<MetaTypeID, RuleCollection>; // std::map<...>;

			// The name of this state.
			std::optional<StringHash> name;

			struct
			{
				// List of component-type descriptions to be inherited from a previous state.
				// (Or added, if not yet present when switching to this state)
				MetaDescription persist;

				// List of component-type descriptions to be added or reconstructed.
				// 
				// All components described will be removed automatically upon switching to another state,
				// unless either this state's `persist` description, or the new state's `persist` description includes the component.
				MetaDescription add;

				// List of component-types to be explicitly removed upon switching to this state.
				MetaRemovalDescription remove;

				// List of component-types to be frozen while this state is active.
				MetaStorageDescription freeze;

				// List of component-types to be stored while this state is inactive.
				MetaStorageDescription store;
			} components;

			Rules rules;

			// Executes appropriate add/remove/persist functions in order to establish this state as current.
			void update(Registry& registry, Entity entity, EntityStateIndex self_index, const EntityState* previous=nullptr, std::optional<EntityStateIndex> prev_index=std::nullopt) const;

			// Utility function for building the `components.remove` collection.
			std::size_t build_removals(const util::json& removal_list, bool cross_reference_persist=false);

			// Utility function for building the `components.freeze` collection.
			std::size_t build_frozen(const util::json& frozen_list, bool cross_reference_persist=true);

			// Utility function for building the `components.store` collection.
			std::size_t build_storage(const util::json& storage_list, bool cross_reference_persist=true);

			// Subroutine of `build_type_list`, meant to handle individual entries, rather than JSON arrays.
			// This method returns true if a component entry could be processed from `list_entry`.
			bool process_type_list_entry(MetaIDStorage& types_out, const util::json& list_entry, bool cross_reference_persist);

			// Convenience overload that bypasses JSON-to-string conversion.
			bool process_type_list_entry(MetaIDStorage& types_out, std::string_view list_entry, bool cross_reference_persist);

			// Shared processing routine for resolving `MetaIDStorage` objects from a JSON array.
			std::size_t build_type_list(const util::json& type_names, MetaIDStorage& types_out, bool cross_reference_persist);

			const RuleCollection* get_rules(MetaTypeID type_id) const;
		protected:

			bool add_component(Registry& registry, Entity entity, const MetaTypeDescriptor& component) const;

			bool remove_component(Registry& registry, Entity entity, const MetaType& type) const;
			bool remove_component(Registry& registry, Entity entity, const MetaTypeDescriptor& component) const;

			bool has_component(Registry& registry, Entity entity, const MetaType& type) const;

			// Returns false if the component does not currently exist for `entity`.
			// If `value_assignment` is false, no value-assignment actions will be performed. (Status only)
			bool update_component_fields(Registry& registry, Entity entity, const MetaTypeDescriptor& component, bool value_assignment=true, bool direct_modify=false) const;

			void decay(Registry& registry, Entity entity, EntityStateIndex self_index, const MetaDescription* next_state_persist=nullptr) const;

			// Adds components to `entity` in `registry`.
			// Added components are removed during `decay`.
			void add(Registry& registry, Entity entity) const;
			void remove(Registry& registry, Entity entity) const;
			void persist(Registry& registry, Entity entity, bool value_assignment=true) const;

			FrozenStateComponent& freeze(Registry& registry, Entity entity, EntityStateIndex self_index) const;
			FrozenStateComponent& unfreeze(Registry& registry, Entity entity, EntityStateIndex self_index) const;
			
			StateStorageComponent& store(Registry& registry, Entity entity, EntityStateIndex self_index) const;
			StateStorageComponent& retrieve(Registry& registry, Entity entity, EntityStateIndex self_index) const;
	};
}