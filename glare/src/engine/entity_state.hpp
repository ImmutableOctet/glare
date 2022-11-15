#pragma once

#include "meta/types.hpp"
#include "meta/meta_description.hpp"

// TODO: Forward declare JSON type.
#include <util/json.hpp>

#include <optional>

namespace engine
{
	class EntityState
	{
		public:
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
			} components;

			// Executes appropriate add/remove/persist functions in order to establish this state as current.
			void update(Registry& registry, Entity entity, const EntityState* previous=nullptr) const;

			// Utility function for building `components.remove` collection.
			void build_removals(const util::json& removal_list, bool cross_reference_persist=false);
		protected:
			bool add_component(Registry& registry, Entity entity, const MetaTypeDescriptor& component) const;

			bool remove_component(Registry& registry, Entity entity, const MetaType& type) const;
			bool remove_component(Registry& registry, Entity entity, const MetaTypeDescriptor& component) const;

			bool has_component(Registry& registry, Entity entity, const MetaType& type) const;

			// Returns false if the component does not currently exist for `entity`.
			// If `value_assignment` is false, no value-assignment actions will be performed. (Status only)
			bool update_component_fields(Registry& registry, Entity entity, const MetaTypeDescriptor& component, bool value_assignment=true) const;

			void decay(Registry& registry, Entity entity, const MetaDescription* next_state_persist=nullptr) const;
			void add(Registry& registry, Entity entity) const;
			void remove(Registry& registry, Entity entity) const;
			void persist(Registry& registry, Entity entity, bool value_assignment=true) const;

			// Old notes:

			// Persistent components; components that remain between state transitions,
			// as long as the transitioned-to state has the state in some way as well.
			//TypeInfo transient_type_info;

			// Temporary components; local only to the current state (temporary).
			//TypeInfo temporary_type_info;

			/*
				Types of components we want:

				* Static instantiation for the entity. (instantiated once, during entity construction)

				* Local copy that is reinitialized on state start.

				* Instance is shared between states. (i.e. control, gravity, etc.) -- not to be confused with 'static' components.

				* Local copy that is frozen while in another state,
				but remains indefinitely, able to 'wake' when coming back to its originating state. (requires dynamic storage)
			*/
	};
}