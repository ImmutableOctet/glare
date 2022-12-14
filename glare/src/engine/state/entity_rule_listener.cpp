#include "entity_rule_listener.hpp"

#include "components/state_component.hpp"
#include "commands/state_change_command.hpp"

#include <engine/entity_state.hpp>
#include <engine/timer.hpp>

#include <engine/meta/meta.hpp>

#include <engine/commands/component_patch_command.hpp>
#include <engine/commands/component_replace_command.hpp>

#include <engine/components/player_component.hpp>
#include <engine/components/player_target_component.hpp>
#include <engine/components/instance_component.hpp>

#include <engine/resource_manager/resource_manager.hpp>

#include <util/variant.hpp>

// Debugging related:
#include <util/log.hpp>
//#include <engine/command.hpp>

namespace engine
{
	EntityRuleListener::EntityRuleListener(Service* service)
		: MetaEventListener(service) {}

	std::size_t EntityRuleListener::count_active_rules() const
	{
		std::size_t count = 0;

		for (const auto& entry : active_rules)
		{
			if ((entry.state) && (entry.reference_count > 0))
			{
				count++;
			}
		}

		return count;
	}

	bool EntityRuleListener::has_active_rule() const
	{
		for (const auto& entry : active_rules)
		{
			if ((entry.state) && (entry.reference_count > 0))
			{
				return true;
			}
		}

		return false;
	}

	bool EntityRuleListener::add_rules(const EntityState& state)
	{
		for (auto& entry : active_rules)
		{
			if (entry.state == &state)
			{
				entry.reference_count++;

				return false;
			}
		}

		active_rules.emplace_back(&state, 1);

		return true;
	}

	bool EntityRuleListener::add_rules(const EntityState& state, MetaTypeID event_type_id)
	{
		if (!state.get_rules(event_type_id))
		{
			return false;
		}

		return add_rules(state);
	}

	bool EntityRuleListener::remove_rules(const EntityState& state)
	{
		std::size_t idx = 0;

		for (auto& entry : active_rules)
		{
			if (entry.state == &state)
			{
				if ((--entry.reference_count) == 0) // <=
				{
					active_rules.erase((active_rules.begin() + idx));

					return true;
				}

				return false;
			}

			idx++;
		}

		return true;
	}

	bool EntityRuleListener::contains(const EntityState* state) const
	{
		if (!state)
		{
			return false;
		}

		for (auto& entry : active_rules)
		{
			if (entry.state == state)
			{
				return true;
			}
		}

		return false;
	}

	bool EntityRuleListener::contains(const EntityState& state) const
	{
		return contains(&state);
	}

	bool EntityRuleListener::on_disconnect(Service* service, const entt::meta_type& type)
	{
		if (!is_active())
		{
			return false;
		}

		if (!has_active_rule())
		{
			return MetaEventListener::on_disconnect(service, type);
		}

		return false; // (this->service != service);
	}

	void EntityRuleListener::on_event(const entt::meta_type& type, entt::meta_any event_instance)
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

		const auto type_id = event_instance.type().id();

		auto& registry = service->get_registry();
		const auto& resource_manager = service->get_resource_manager();

		auto stateful_entities = registry.view<StateComponent, InstanceComponent>();

		const auto event_player_index = resolve_player_index(event_instance);

		// TODO: Determine if this lookup would be better handled through an implementation-defined component type. (e.g. pointer-to-state as a component)
		stateful_entities.each([this, &registry, type_id, &resource_manager, &event_instance, event_player_index](Entity entity, const StateComponent& state_comp, const InstanceComponent& instance_comp)
		{
			const auto& state_index = state_comp.state_index;
			const auto& instance_path = instance_comp.instance;

			const auto* descriptor = resource_manager.get_existing_descriptor(instance_comp.instance);

			assert(descriptor);

			if (!descriptor)
			{
				return;
			}

			const auto& state = *descriptor->states.at(state_index);

			const auto* rules = state.get_rules(type_id);

			if (!rules)
			{
				return;
			}

			// Determine if this is a player-specific event type:
			if (event_player_index)
			{
				// Check if this entity has a `PlayerComponent` attached:
				if (const auto* player_comp = registry.try_get<PlayerComponent>(entity))
				{
					const auto entity_player_index = player_comp->player_index;

					if (event_player_index != entity_player_index)
					{
						return;
					}
				}
				// Check if this entity has a `PlayerTargetComponent` attached:
				else if (const auto* player_comp = registry.try_get<PlayerTargetComponent>(entity))
				{
					const auto entity_player_index = player_comp->player_index;

					if (event_player_index != entity_player_index)
					{
						return;
					}
				}
			}

			using Target      = EntityTarget;
			using Transition  = EntityStateTransitionAction;
			using Command     = EntityStateCommandAction;
			using StateUpdate = EntityStateUpdateAction;

			for (const auto& rule_entry : *rules)
			{
				const auto& condition = rule_entry.condition;
				const auto& delay = rule_entry.delay;

				// Resolve trigger condition status:
				if (condition)
				{
					if (!EventTriggerConditionType::get_condition_status(*condition, event_instance, registry, entity))
					{
						// Condition has not been met; continue to next rule.
						return;
					}
				}

				const auto target = rule_entry.resolve_target(registry, entity);

				if (target == null)
				{
					print_warn("Failed to resolve target entity.");

					continue;
				}

				util::visit
				(
					rule_entry.action,
					
					[this, entity, target, &delay](const Transition& transition)
					{
						service->timed_event<StateChangeCommand>(delay, entity, target, transition.state_name);
					},

					[this, &registry, entity, target, &delay](const Command& command)
					{
						auto command_instance = command.command.instance(true, registry, entity);

						set_data_member(command_instance, "source", entity);
						set_data_member(command_instance, "target", target);

						// Debugging related:
						//const auto* cmd = reinterpret_cast<const engine::Command*>(command_instance.data());

						service->timed_event(delay, std::move(command_instance));
					},

					[this, &registry, entity, target, &delay](const StateUpdate& state_update)
					{
						for (const auto& component : state_update.updated_components.type_definitions)
						{
							if (!component.forces_field_assignment())
							{
								auto instance = component.instance(true, registry, entity);

								if (instance)
								{
									// Replace/reconstruct the component according to the desired specifications.
									service->timed_event<ComponentReplaceCommand>(delay, entity, target, std::move(instance));

									continue;
								}
								else
								{
									//print_warn("Failed to instantiate component #{} during update procedure.", component.type.id());

									// Continue to the 'patch' control-path instead.
								}
							}

							// Attempt to patch the component in-place, rather than reconstructing it.
							// NOTE: Raw pointer type. (May replace with `std::shared_ptr` later)
							service->timed_event<ComponentPatchCommand>(delay, entity, target, &component);
						}
					}
				);
			}
		});
	}
}