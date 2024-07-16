#pragma once

#include "entity_state_rule.hpp"

#include <engine/meta/meta_event_listener.hpp>
#include <engine/meta/meta_evaluation_context.hpp>

#include <util/small_vector.hpp>

#include <optional>

namespace engine
{
	class Service;
	class ResourceManager;
	class EntityState;
	class EntityDescriptor;

	struct EntityThread;
	struct EntityThreadDescription;
	struct EntityThreadComponent;
	struct EntityInstruction;

	class EntityListener : public MetaEventListener
	{
		public:
			using ReferenceCount = std::uint16_t;
			using RuleCollection = EntityStateRuleCollection;

			EntityListener(Service* service=nullptr, SystemManagerInterface* system_manager=nullptr);

			bool add_entity(Entity entity);
			bool remove_entity(Entity entity, ReferenceCount references_to_remove=1, bool force=false);

			bool contains(Entity entity) const;

			void on_event(const MetaType& type, MetaAny event_instance) override;

		protected:
			struct Reference
			{
				Entity         entity          = null;
				ReferenceCount reference_count = 1;
			};

			bool has_listening_entity() const;

			bool on_disconnect(Service* service, const MetaType& type) override;
			
			void on_component_create(Registry& registry, Entity entity, const MetaAny& component) override;
			void on_component_update(Registry& registry, Entity entity, const MetaAny& component) override;
			void on_component_destroy(Registry& registry, Entity entity, const MetaAny& component) override;

			// A collection of entities listening for the
			// event-type encapsulated by this listener instance.
			util::small_vector<Reference, 8> listening_entities; // 16

		private:
			void update_entity
			(
				Registry& registry, Entity entity,
				const EntityDescriptor& descriptor,
				const MetaAny& event_instance
			);

			void update_entity_conditional_yield
			(
				Registry& registry, Entity entity,
				const EntityDescriptor& descriptor,

				const EntityThreadDescription& thread_data,
				const EntityInstruction& current_instruction,
				
				EntityThreadComponent& thread_comp, EntityThread& thread,

				const MetaAny& event_instance,

				const MetaEvaluationContext& evaluation_context={}
			);

			void update_entity_coroutine_yield
			(
				Registry& registry, Entity entity,
				const EntityDescriptor& descriptor,

				const EntityThreadDescription& thread_data,
				const EntityInstruction& current_instruction,

				EntityThreadComponent& thread_comp, EntityThread& thread,

				const MetaAny& event_instance,

				const MetaEvaluationContext& evaluation_context={}
			);

			void update_entity_coroutine_yield
			(
				Registry& registry, Entity entity,
				const EntityDescriptor& descriptor,

				EntityThreadComponent& thread_comp, EntityThread& thread,

				const MetaAny& event_instance,

				const MetaEvaluationContext& evaluation_context={}
			);

			void handle_state_rules
			(
				Registry& registry, Entity entity,
				const EntityDescriptor& descriptor,
				const EntityStateRuleCollection& rules,
				
				const MetaAny& event_instance,
				
				const MetaEvaluationContext& evaluation_context={}
			);
	};
}