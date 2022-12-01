#include "entity_rule_listener.hpp"

#include "components/state_component.hpp"
#include "commands/state_change_command.hpp"

#include <engine/entity_state.hpp>
#include <engine/timer.hpp>

#include <engine/meta/meta.hpp>

#include <engine/components/player_component.hpp>
#include <engine/components/player_target_component.hpp>
#include <engine/components/relationship_component.hpp>
#include <engine/components/name_component.hpp>
#include <engine/components/instance_component.hpp>

#include <engine/resource_manager/resource_manager.hpp>

#include <util/variant.hpp>

// Debugging related:
#include <util/log.hpp>

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
				if (const auto* player_comp = registry.try_get<PlayerTargetComponent>(entity))
				{
					const auto entity_player_index = player_comp->player_index;

					if (event_player_index != entity_player_index)
					{
						return;
					}
				}
			}

			using Transition = EntityStateTransitionRule;
			using Command    = EntityStateCommandRule;

			for (const auto& rule_entry : *rules)
			{
				const auto& condition = rule_entry.condition;
				const auto& delay = rule_entry.delay;

				util::visit
				(
					rule_entry.action,
					
					[this, &registry, entity, event_player_index, &event_instance, &condition, &delay](const Transition& transition)
					{
						// Resolve trigger condition status:
						if (condition)
						{
							if (!condition->condition_met(event_instance))
							{
								// Condition has not been met; continue to next rule.
								return;
							}
						}

						Entity target = null;

						util::visit
						(
							transition.target,

							[entity, &target](const Transition::SelfTarget&)
							{
								target = entity;
							},

							[&registry, entity, &target](const Transition::ParentTarget&)
							{
								if (const RelationshipComponent* relationship = registry.try_get<RelationshipComponent>(entity))
								{
									target = relationship->get_parent();
								}
							},

							[&target](const Transition::EntityTarget& exact_entity)
							{
								target = exact_entity.entity;
							},

							[&registry, &target](const Transition::EntityNameTarget& named_target)
							{
								auto named_range = registry.view<NameComponent>().each();

								for (auto it = named_range.begin(); it != named_range.end(); it++)
								{
									const auto& [e_out, name] = *it;

									// First available match.
									if (name.hash() == named_target.entity_name)
									{
										target = e_out;

										return;
									}
								}
							},

							[&registry, entity, &target](const Transition::ChildTarget& child_target)
							{
								//assert(child_target.child_name);

								const RelationshipComponent* relationship = registry.try_get<RelationshipComponent>(entity);

								if (!relationship)
								{
									return;
								}

								relationship->enumerate_children
								(
									registry,
									
									[&registry, &child_target, &target](Entity child, const RelationshipComponent& child_relationship, Entity next_child) -> bool
									{
										const auto* child_name_comp = registry.try_get<NameComponent>(child);

										if (!child_name_comp)
										{
											return true;
										}

										if (child_name_comp->hash() == child_target.child_name)
										{
											target = child;

											return false;
										}
										
										return true;
									},

									child_target.recursive
								);
							},
							
							[&registry, &target](const Transition::PlayerTarget& player_target)
							{
								auto player_range = registry.view<PlayerComponent>().each();

								for (auto it = player_range.begin(); it != player_range.end(); it++)
								{
									const auto& [e_out, player_comp] = *it;

									// First available match.
									if (player_comp.player_index == player_target.player_index)
									{
										target = e_out;

										return;
									}
								}
							}
						);

						if (target == null)
						{
							print_warn("Failed to resolve state transition.");
						}
						else
						{
							service->timed_event<StateChangeCommand>(delay, entity, target, transition.state_name);
						}
					},

					[&event_instance](const Command& command)
					{
						// TODO: Implement command generation.
					}
				);
			}
		});
	}
}