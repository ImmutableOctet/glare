#pragma once

#include <engine/types.hpp>
#include <engine/entity/entity_state_rule.hpp>
#include <engine/meta/meta_event_listener.hpp>

#include <util/small_vector.hpp>

namespace engine
{
	class Service;
	class EntityState;

	class EntityListener : public MetaEventListener
	{
		public:
			using RuleCollection = EntityStateRuleCollection;

			EntityListener(Service* service=nullptr);

			std::size_t count_active_rules() const;
			bool has_active_rule() const;

			// The return value of this method indicates if a new `Reference` was allocated internally.
			bool add_rules(const EntityState& state);

			// The return value of this method indicates if a new `Reference` was allocated internally.
			bool add_rules(const EntityState& state, MetaTypeID event_type_id);

			// This method returns true if there are no longer any references to `state` internally.
			bool remove_rules(const EntityState& state);

			// Checks if `state` is currently being referenced by this listener.
			bool contains(const EntityState* state) const;

			// Checks if `state` is currently being referenced by this listener.
			bool contains(const EntityState& state) const;
		protected:
			struct Reference
			{
				//const RuleCollection* rules = nullptr; // const RuleCollection&;
				const EntityState* state = nullptr;

				std::uint16_t reference_count = 0;
			};

			// TODO: Look into whether we want to use either the
			// pointer-to-state approach, or the descriptor lookup approach.
			util::small_vector<Reference, 8> active_rules;

			bool on_disconnect(Service* service, const entt::meta_type& type) override;

			void on_event(const entt::meta_type& type, entt::meta_any event_instance) override;
	};
}