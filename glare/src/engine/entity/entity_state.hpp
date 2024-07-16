#pragma once

#include "types.hpp"

#include "entity_state_rule.hpp"
#include "entity_thread_range.hpp"
#include "meta_description.hpp"

#include <engine/meta/types.hpp>
#include <engine/timer.hpp>

// TODO: Forward declare JSON type.
#include <util/json.hpp>

#include <util/small_vector.hpp>

#include <optional>
#include <tuple>
#include <string_view>
#include <unordered_map>
//#include <map>

namespace engine
{
	struct FrozenStateComponent;
	struct StateStorageComponent;

	class MetaParsingContext;

	class EntityDescriptor;

	class EntityState
	{
		public:
			using RuleCollection = EntityStateRuleCollection;

			// TODO: Review whether it makes sense to pair keys and values in a `small_vector` of tuples, rather than a map.
			using Rules = std::unordered_map<MetaTypeID, RuleCollection>; // std::map<...>;

			using ImmediateThreadDetails = EntityThreadRange;

			// The name of this state.
			// TODO: Remove use of `std::optional` in favor of zero/default value.
			std::optional<EntityStateHash> name;

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

				// List of component-types to be copied/cloned upon initial activation of this state.
				MetaStorageDescription init_copy;

				// List of component-types to be copied/cloned every time this state is activated.
				// After initial activation, these components will persist with the entity while this state is active.
				MetaStorageDescription local_copy;
			} components;

			Rules rules;

			// Delay in 'activation' portion of state initialization.
			std::optional<Timer::Duration> activation_delay; // = Timer::Duration::zero(); // Timer::Duration

			// TODO: Add method to change this from JSON.
			struct
			{
				bool remove_add_components        : 1 = true;
				bool keep_modified_add_components : 1 = true;
			} decay_policy;

			util::small_vector<ImmediateThreadDetails, 1> immediate_threads; // 2

			// Executes appropriate add/remove/persist functions in order to establish this state as current.
			void update(const EntityDescriptor& descriptor, Registry& registry, Entity entity, EntityStateIndex self_index, const EntityState* previous=nullptr, std::optional<EntityStateIndex> prev_index=std::nullopt, bool decay_prev_state=true, bool update_state_component=true) const;

			// Decays this state from `entity`, but does not perform any additional activation/initialization of the next state.
			//
			// NOTE: This is normally called automatically as a subroutine of `update`.
			// For most use-cases, `update` is a more appropriate interface.
			void decay(const EntityDescriptor& descriptor, Registry& registry, Entity entity, EntityStateIndex self_index, const MetaDescription* next_state_persist=nullptr) const;

			// Activates this state for `entity`, but does not perform
			// any decay or cleanup operations for the previous state.
			//
			// NOTE: This is normally called automatically as a subroutine of `update`.
			// For most use-cases, `update` is a more appropriate interface.
			void activate(const EntityDescriptor& descriptor, Registry& registry, Entity entity, EntityStateIndex self_index, std::optional<EntityStateIndex> prev_index=std::nullopt, bool update_state_component=true) const;

			// Manually updates the `StateComponent` instance associated to `entity`.
			void force_update_component(Registry& registry, Entity entity, EntityStateIndex self_index, std::optional<EntityStateIndex> prev_index=std::nullopt) const;

			// Utility function for building the `components.remove` collection.
			std::size_t build_removals(const EntityDescriptor& descriptor, const util::json& removal_list, const MetaParsingContext& opt_parsing_context={}, bool cross_reference_persist=false);

			// Utility function for building the `components.freeze` collection.
			std::size_t build_frozen(const EntityDescriptor& descriptor, const util::json& frozen_list, const MetaParsingContext& opt_parsing_context={}, bool cross_reference_persist=true);

			// Utility function for building the `components.store` collection.
			std::size_t build_storage(const EntityDescriptor& descriptor, const util::json& storage_list, const MetaParsingContext& opt_parsing_context={}, bool cross_reference_persist=true);

			// Utility function for building the `components.local_copy`
			// collection, as well as appending to `components.freeze`.
			std::size_t build_local_copy(const EntityDescriptor& descriptor, const util::json& local_copy_list, const MetaParsingContext& opt_parsing_context={}, bool cross_reference_persist=false); // true

			// Utility function for building the `components.init_copy` collection,
			// as well as appending to `components.freeze` and `components.store`.
			std::size_t build_init_copy(const EntityDescriptor& descriptor, const util::json& init_copy_list, const MetaParsingContext& opt_parsing_context={}, bool cross_reference_persist=false); // true

			// Subroutine of `build_type_list`, meant to handle individual entries, rather than JSON arrays.
			// This method returns true if a component entry could be processed from `list_entry`.
			bool process_type_list_entry(const EntityDescriptor& descriptor, MetaIDStorage& types_out, const util::json& list_entry, bool cross_reference_persist, const MetaParsingContext& opt_parsing_context={});

			// Convenience overload that bypasses JSON-to-string conversion.
			bool process_type_list_entry(const EntityDescriptor& descriptor, MetaIDStorage& types_out, std::string_view list_entry, bool cross_reference_persist, const MetaParsingContext& opt_parsing_context={});

			// Shared processing routine for resolving `MetaIDStorage` objects from a JSON array.
			std::size_t build_type_list(const EntityDescriptor& descriptor, const util::json& type_names, MetaIDStorage& types_out, bool cross_reference_persist, const MetaParsingContext& opt_parsing_context={});

			const RuleCollection* get_rules(MetaTypeID type_id) const;

			inline bool has_activation_delay() const
			{
				//return (activation_delay != Timer::Duration::zero());
				return activation_delay.has_value();
			}
		protected:

			//bool add_component(Registry& registry, Entity entity, const MetaTypeDescriptor& component) const;

			bool remove_component(Registry& registry, Entity entity, const MetaType& type) const;
			bool remove_component(Registry& registry, Entity entity, const MetaTypeDescriptor& component) const;

			bool has_component(Registry& registry, Entity entity, const MetaType& type) const;

			// Adds components to `entity` in `registry`.
			// Added components are removed during `decay`.
			void add(const EntityDescriptor& descriptor, Registry& registry, Entity entity) const;
			void remove(Registry& registry, Entity entity) const;
			void persist(const EntityDescriptor& descriptor, Registry& registry, Entity entity, bool value_assignment=true) const;

			void activate_threads(const EntityDescriptor& descriptor, Registry& registry, Entity entity, EntityStateIndex self_index) const;
			void decay_threads(const EntityDescriptor& descriptor, Registry& registry, Entity entity, EntityStateIndex self_index) const;

			FrozenStateComponent& freeze(Registry& registry, Entity entity, EntityStateIndex self_index) const;
			FrozenStateComponent& unfreeze(Registry& registry, Entity entity, EntityStateIndex self_index) const;
			
			StateStorageComponent& store(Registry& registry, Entity entity, EntityStateIndex self_index) const;
			StateStorageComponent& retrieve(Registry& registry, Entity entity, EntityStateIndex self_index) const;

			StateStorageComponent& copy(Registry& registry, Entity entity, EntityStateIndex self_index) const;
	};
}