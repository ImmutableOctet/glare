#include "entity_listener.hpp"
#include "entity_state_action.hpp"

#include "components/instance_component.hpp"
#include "components/state_component.hpp"
#include "components/entity_thread_component.hpp"

#include <engine/entity/entity_state.hpp>
#include <engine/timer.hpp>

#include <engine/meta/meta.hpp>
#include <engine/meta/events.hpp>

#include <engine/components/player_component.hpp>
#include <engine/components/player_target_component.hpp>

#include <engine/resource_manager/resource_manager.hpp>

#include <util/variant.hpp>

#include <algorithm>

// Debugging related:
#include <util/log.hpp>
//#include <engine/command.hpp>

namespace engine
{
	EntityListener::EntityListener(Service* service)
		: MetaEventListener(service) {}

	bool EntityListener::add_entity(Entity entity)
	{
		for (auto it = listening_entities.begin(); it != listening_entities.end(); it++)
		{
			auto& listener_entry = *it;

			if (listener_entry.entity == entity)
			{
				listener_entry.reference_count++;

				return false;
			}
		}

		listening_entities.emplace_back(entity);

		return true;
	}

	bool EntityListener::remove_entity(Entity entity, bool force)
	{
		for (auto it = listening_entities.begin(); it != listening_entities.end(); it++)
		{
			auto& listener_entry = *it;

			if (listener_entry.entity == entity)
			{
				if (force || ((--listener_entry.reference_count) == 0)) // <=
				{
					listening_entities.erase(it);

					// Exit immediately, since we've removed the specified `entity`.
					return true;
				}

				// Exit immediately, since we've found the intended `entity`,
				// but only needed to decrement its reference-count.
				return false;
			}
		}

		return false;
	}

	bool EntityListener::contains(Entity entity) const
	{
		if (entity == null)
		{
			return false;
		}

		for (const auto& entry : listening_entities)
		{
			if (entry.entity == entity)
			{
				return true;
			}
		}

		return false;
	}

	bool EntityListener::has_listening_entity() const
	{
		return !listening_entities.empty();
	}

	bool EntityListener::on_disconnect(Service* service, const MetaType& type)
	{
		if (!is_active())
		{
			return false;
		}

		if (!has_listening_entity())
		{
			return MetaEventListener::on_disconnect(service, type);
		}

		return false; // (this->service != service);
	}

	void EntityListener::on_event(const MetaType& type, MetaAny event_instance)
	{
		assert(service);

		if (!service)
		{
			return;
		}

		assert(event_instance);

		if (!event_instance)
		{
			return;
		}

		assert(type.id() == this->type_id);
		assert(event_instance.type().id() == this->type_id);

		auto& registry = service->get_registry();
		const auto& resource_manager = service->get_resource_manager();

		const auto event_player_index = resolve_player_index(event_instance);

		for (const auto& listener_entry : listening_entities)
		{
			const auto entity = listener_entry.entity;

			const auto* instance_comp = registry.try_get<InstanceComponent>(entity);

			if (!instance_comp)
			{
				continue;
			}

			// Determine if this is a player-specific event type:
			if (event_player_index)
			{
				if (*event_player_index != ANY_PLAYER)
				{
					// Check if this entity has a `PlayerComponent` attached:
					if (const auto* player_comp = registry.try_get<PlayerComponent>(entity))
					{
						const auto entity_player_index = player_comp->player_index;

						if (*event_player_index != entity_player_index)
						{
							continue;
						}
					}
					// Check if this entity has a `PlayerTargetComponent` attached:
					else if (const auto* player_comp = registry.try_get<PlayerTargetComponent>(entity))
					{
						const auto entity_player_index = player_comp->player_index;

						if (*event_player_index != entity_player_index)
						{
							continue;
						}
					}
				}
			}

			const auto& instance_path = instance_comp->instance;
			const auto* descriptor = resource_manager.get_existing_descriptor(instance_path);

			assert(descriptor);

			if (!descriptor)
			{
				continue;
			}

			update_entity
			(
				registry, entity,
				*descriptor,
				event_instance
			);
		}
	}

	void EntityListener::update_entity
	(
		Registry& registry, Entity entity,
		const EntityDescriptor& descriptor,
		const MetaAny& event_instance
	)
	{
		using namespace engine::instructions;

		if (const auto state_comp = registry.try_get<StateComponent>(entity))
		{
			const auto& state_index = state_comp->state_index;
			const auto& state = *(descriptor.states[state_index]);

			if (const auto rules = state.get_rules(this->type_id))
			{
				handle_state_rules
				(
					registry, entity, descriptor,
					*rules,
					event_instance
				);
			}
		}

		if (auto thread_comp = registry.try_get<EntityThreadComponent>(entity))
		{
			// Get current instruction to see if its condition has been met.
			// If the condition is met, attempt to remove this entity from this listener.
			// (Decrement reference count, etc.)

			auto& active_threads = thread_comp->threads;

			for (auto& thread : active_threads)
			{
				if (!thread.is_yielding)
				{
					continue;
				}

				const auto& thread_data = descriptor.get_thread(thread.thread_index);

				const auto& current_instruction = thread_data.get_instruction(thread.next_instruction);

				// Ensure this thread is actually yielding.
				const auto& yield_instruction = std::get<Yield>(current_instruction.value);

				const auto& yield_condition = yield_instruction.condition.get(descriptor);

				bool yield_condition_met = false;

				EventTriggerConditionType::visit_type_enabled
				(
					yield_condition,

					[this, &registry, entity, &event_instance, &yield_condition_met](const auto& condition)
					{
						if (condition.has_type_compatible(this->type_id))
						{
							yield_condition_met = condition.condition_met(event_instance, registry, entity);
						}
					}
				);

				if (yield_condition_met)
				{
					// TODO: Move to dedicated function.
					thread.is_yielding = false;
					thread.next_instruction++;

					// See `EntitySystem::step_thread` for corresponding `add_entity` usage prior.
					remove_entity(entity);
				}
			}
		}
	}

	void EntityListener::handle_state_rules
	(
		Registry& registry, Entity entity,
		const EntityDescriptor& descriptor,
		const EntityStateRuleCollection& rules,
		const MetaAny& event_instance
	)
	{
		for (const auto& rule_entry : rules)
		{
			const auto& condition = rule_entry.condition;

			// Resolve trigger condition status:
			if (condition)
			{
				if (!EventTriggerConditionType::get_condition_status(*condition, event_instance, registry, entity))
				{
					// Condition has not been met; continue to next rule.
					continue;
				}
			}

			const auto& delay = rule_entry.delay;

			assert(service);

			execute_action
			(
				registry,
				*service,
				descriptor,
				rule_entry.action,
				entity, rule_entry.target,
				delay
			);
		}
	}

	void EntityListener::on_component_create(Registry& registry, Entity entity, const MetaAny& component)
	{
		service->event<OnComponentCreate>(entity, component.as_ref()); // component // entt::forward_as_meta(component)
	}

	void EntityListener::on_component_update(Registry& registry, Entity entity, const MetaAny& component)
	{
		service->event<OnComponentUpdate>(entity, component.as_ref());
	}

	void EntityListener::on_component_destroy(Registry& registry, Entity entity, const MetaAny& component)
	{
		service->event<OnComponentDestroy>(entity, component.as_ref());
	}
}