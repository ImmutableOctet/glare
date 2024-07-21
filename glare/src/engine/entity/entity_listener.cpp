#include "entity_listener.hpp"
#include "entity_state_action.hpp"
#include "entity_instruction.hpp"
#include "entity_descriptor.hpp"

#include "entity_system.hpp"

#include "components/instance_component.hpp"
#include "components/state_component.hpp"
#include "components/entity_thread_component.hpp"

#include <engine/timer.hpp>

#include <engine/entity/entity_state.hpp>
#include <engine/entity/entity_thread.hpp>

#include <engine/meta/data_member.hpp>
#include <engine/meta/events.hpp>

#include <engine/script/script_handle.hpp>
#include <engine/script/script.hpp>

#include <engine/components/player_component.hpp>
#include <engine/components/player_target_component.hpp>

//#include <engine/resource_manager/resource_manager.hpp>

#include <util/variant.hpp>

#include <algorithm>
#include <variant>

#include <cassert>

// Debugging related:
#include <util/log.hpp>
//#include <engine/command.hpp>

namespace engine
{
	EntityListener::EntityListener(Service* service, SystemManagerInterface* system_manager)
		: MetaEventListener(service, system_manager) {}

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

	bool EntityListener::remove_entity(Entity entity, ReferenceCount references_to_remove, bool force)
	{
		for (auto it = listening_entities.begin(); it != listening_entities.end(); it++)
		{
			auto& listener_entry = *it;

			if (listener_entry.entity == entity)
			{
				if (force)
				{
					listening_entities.erase(it);

					// Exit immediately, since we've removed the specified `entity`.
					return true;
				}
				else
				{
					if (listener_entry.reference_count > references_to_remove)
					{
						listener_entry.reference_count -= references_to_remove;

						// Exit immediately, since we've found the intended `entity`,
						// but only needed to decrement its reference-count.
						return false;
					}
					else
					{
						//listener_entry.reference_count = 0;

						listening_entities.erase(it);

						// Exit immediately, since we've removed the specified `entity`.
						return true;
					}
				}
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
		//const auto& resource_manager = service->get_resource_manager();

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

			const auto& descriptor = instance_comp->get_descriptor();

			update_entity
			(
				registry, entity,
				descriptor,
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

		auto evaluation_context = MetaEvaluationContext
		{
			.variable_context = {},
			.service = service,
			.system_manager = system_manager
		};

		if (const auto state_comp = registry.try_get<StateComponent>(entity))
		{
			const auto& state_index = state_comp->state_index;

			// TODO: Implement multi-entity state descriptions. (use of `get(descriptor)`)
			const auto& state = descriptor.states[state_index].get(descriptor);

			if (const auto rules = state.get_rules(this->type_id))
			{
				handle_state_rules
				(
					registry, entity, descriptor,
					*rules,
					event_instance,
					evaluation_context
				);
			}
		}

		if (auto thread_component = registry.try_get<EntityThreadComponent>(entity))
		{
			// Get current instruction to see if its condition has been met.
			// If the condition is met, attempt to remove this entity from this listener.
			// (Decrement reference count, etc.)

			auto& active_threads = thread_component->threads;

			for (auto& thread : active_threads)
			{
				if (!thread.is_yielding)
				{
					continue;
				}

				// TODO: Determine if `thread_index` check is necessary.
				if ((thread.next_instruction == ENTITY_INSTRUCTION_INDEX_INVALID) && (thread.thread_index == ENTITY_THREAD_INDEX_INVALID))
				{
					update_entity_coroutine_yield
					(
						registry, entity, descriptor,
						*thread_component, thread,
						event_instance,
						evaluation_context
					);
				}
				else
				{
					const auto& thread_data = descriptor.get_thread(thread.thread_index);

					const auto& current_instruction = thread_data.get_instruction(thread.next_instruction);
				
					using InstructionVariant = EntityInstruction::InstructionType;

					switch (current_instruction.type_index())
					{
						case util::variant_index<InstructionVariant, Yield>():
							update_entity_conditional_yield
							(
								registry, entity, descriptor,
								thread_data, current_instruction,
								*thread_component, thread,
								event_instance,
								evaluation_context
							);

							break;

						case util::variant_index<InstructionVariant, FunctionCall>():
						case util::variant_index<InstructionVariant, CoroutineCall>():
						case util::variant_index<InstructionVariant, AdvancedMetaExpression>():
							update_entity_coroutine_yield
							(
								registry, entity, descriptor,
								thread_data, current_instruction,
								*thread_component, thread,
								event_instance,
								evaluation_context
							);

							break;
					}
				}
			}
		}
	}

	void EntityListener::update_entity_conditional_yield
	(
		Registry& registry, Entity entity,
		const EntityDescriptor& descriptor,

		const EntityThreadDescription& thread_data,
		const EntityInstruction& current_instruction,

		EntityThreadComponent& thread_component, EntityThread& thread,

		const MetaAny& event_instance,

		const MetaEvaluationContext& evaluation_context
	)
	{
		using namespace engine::instructions;

		// Ensure this thread is actually yielding.
		const auto& yield_instruction = std::get<Yield>(current_instruction.value);

		const auto& yield_condition = yield_instruction.condition.get(descriptor);

		ReferenceCount condition_reference_count = 0;

		bool yield_condition_met = false;

		EventTriggerConditionType::visit_type_enabled
		(
			yield_condition,

			[this, &registry, entity, &descriptor, &event_instance, &evaluation_context, &condition_reference_count, &yield_condition_met](const auto& condition)
			{
				if (condition.has_type_compatible(descriptor, this->type_id))
				{
					yield_condition_met = (yield_condition_met || condition.condition_met(event_instance, registry, entity, evaluation_context));

					condition_reference_count++;
				}
			}
		);

		if (yield_condition_met)
		{
			// Advancement initially set to one to account for the active (yield) instruction.
			EntityInstructionCount instruction_advance = 1;

			if (event_instance)
			{
				auto projected_instruction_index = (thread.next_instruction + instruction_advance);

				bool event_captured = false;

				while (projected_instruction_index < thread_data.size())
				{
					const auto& instruction_after_yield = thread_data.get_instruction(projected_instruction_index);

					if (const auto* event_capture = std::get_if<instructions::EventCapture>(&instruction_after_yield.value))
					{
						if ((!event_capture->intended_type) || (event_capture->intended_type == event_instance.type().id()))
						{
							auto variable_context = EntitySystem::resolve_variable_context
							(
								{}, // Service unavailable for variable assignment operations at this time.
								&registry, entity,
								&thread_component, &thread,
								{}, // Context to be retrieved automatically from `entity`.
								event_capture->variable_details.scope
							);

							const bool assignment_result = variable_context.set
							(
								event_capture->variable_details.scope,
								event_capture->variable_details.name,
								
								MetaAny { event_instance }
							);


							if (assignment_result)
							{
								event_captured = true;
							}
						}

						projected_instruction_index++;
					}
					else
					{
						break;
					}
				}

				// NOTE: We only bypass the subsequent series of `EventCapture` instructions if
				// at least one of them is compatible and can be 'executed' above.
				// 
				// Otherwise, we simply move forward one instruction (the initial yield)
				// to prevent blocking the thread indefinitely.
				// 
				// i.e. incompatible `EventCapture` instructions are only bypassed when a compatible one is found.
				// See `EntitySystem::step_thread` for the fallback implementation of `EventCapture`.
				if (event_captured)
				{
					const auto projected_advance = (projected_instruction_index - thread.next_instruction);

					instruction_advance = projected_advance;
				}
			}

			// Awaken the thread past the active yield instruction + any subsequent event captures (see above).
			thread.unyield(instruction_advance);

			// Remove a reference to this listener for `entity`.
			// NOTE: See `EntitySystem::step_thread` for corresponding `add_entity` usage prior.
			remove_entity(entity, condition_reference_count);
		}
	}

	void EntityListener::update_entity_coroutine_yield
	(
		Registry& registry, Entity entity,
		const EntityDescriptor& descriptor,

		const EntityThreadDescription& thread_data,
		const EntityInstruction& current_instruction,

		EntityThreadComponent& thread_component, EntityThread& thread,

		const MetaAny& event_instance,

		const MetaEvaluationContext& evaluation_context
	)
	{
		update_entity_coroutine_yield
		(
			registry, entity,
			descriptor,
			thread_component, thread,
			event_instance,
			evaluation_context
		);
	}

	void EntityListener::update_entity_coroutine_yield
	(
		Registry& registry, Entity entity,
		const EntityDescriptor& descriptor,
		
		EntityThreadComponent& thread_component, EntityThread& thread,

		const MetaAny& event_instance,

		const MetaEvaluationContext& evaluation_context
	)
	{
		if (!thread.has_fiber())
		{
			return;
		}

		auto& active_fiber = thread.get_fiber();

		if (!active_fiber.has_script_handle())
		{
			return;
		}

		auto& script = *active_fiber.script;

		const auto event_type = event_instance.type();

		if (!script.waiting_for_event(event_type))
		{
			return;
		}

		if (script.has_yield_continuation_predicate())
		{
			const auto& yield_predicate = script.get_yield_continuation_predicate();

			if (!yield_predicate(script, event_instance))
			{
				return;
			}
		}

		const auto local_index = thread_component.get_local_index(thread);

		const bool can_generate_capture_event = ((service) && (local_index));

		const bool can_process_events_exhaustively = ((can_generate_capture_event) && (active_fiber.process_events_exhaustively()));

		if (can_process_events_exhaustively)
		{
			active_fiber.script->set_captured_event(event_instance.as_ref());
		}
		else
		{
			// Create a copy of the event for the script to later observe.
			active_fiber.script->set_captured_event(MetaAny { event_instance });
		}

		// Advancement set to zero to continue execution of the active fiber/coroutine.
		const EntityInstructionCount instruction_advance = 0;

		// Awaken the thread, allowing it to handle the event.
		thread.unyield(instruction_advance);

		if (can_generate_capture_event)
		{
			assert(service);
			assert(local_index);

			service->event<OnThreadEventCaptured>
			(
				entity,
				
				thread.thread_index, thread.thread_id,
				static_cast<ThreadEvent::LocalThreadIndex>(*local_index),
				thread.next_instruction,

				event_type.id()
			);
		}

		// Remove a reference to this listener for `entity`.
		// NOTE: See `EntitySystem::step_thread` for corresponding `add_entity` usage prior.
		remove_entity(entity);
	}

	void EntityListener::handle_state_rules
	(
		Registry& registry, Entity entity,
		const EntityDescriptor& descriptor,
		const EntityStateRuleCollection& rules,
		const MetaAny& event_instance,
		const MetaEvaluationContext& evaluation_context
	)
	{
		for (const auto& rule_entry : rules)
		{
			const auto& condition = rule_entry.condition;

			// Resolve trigger condition status:
			if (condition)
			{
				if (!EventTriggerConditionType::get_condition_status(condition->get(descriptor), event_instance, registry, entity, evaluation_context))
				{
					// Condition has not been met; continue to next rule.
					continue;
				}
			}

			const auto& delay = rule_entry.delay;

			assert(service);

			// TODO: Determine if it makes sense to resolve an evaluation context for this call.
			execute_action
			(
				registry,
				*service,
				descriptor,
				rule_entry.action,
				entity, rule_entry.target,
				delay,
				evaluation_context,
				event_instance
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