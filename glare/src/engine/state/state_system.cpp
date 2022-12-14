#include "state_system.hpp"

#include "events.hpp"

#include "components/state_component.hpp"
#include "commands/state_change_command.hpp"
#include "commands/state_activation_command.hpp"

#include <engine/components/instance_component.hpp>

#include <engine/service.hpp>
#include <engine/service_events.hpp>
#include <engine/meta/meta.hpp>
#include <engine/entity/entity_descriptor.hpp>
#include <engine/entity/entity_state.hpp>

#include <engine/resource_manager/resource_manager.hpp>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	StateSystem::StateSystem(Service& service, const ResourceManager& resource_manager, bool subscribe_immediately)
		: BasicSystem(service), resource_manager(resource_manager)
	{
		if (subscribe_immediately)
		{
			subscribe();
		}
	}

	bool StateSystem::on_subscribe(Service& service)
	{
		auto& registry = service.get_registry();

		// Registry events:
		registry.on_construct<StateComponent>().connect<&StateSystem::on_state_init>(*this);
		registry.on_update<StateComponent>().connect<&StateSystem::on_state_update>(*this);
		registry.on_destroy<StateComponent>().connect<&StateSystem::on_state_destroyed>(*this);

		// Standard events:
		service.register_event<OnServiceUpdate, &StateSystem::on_update>(*this);
		service.register_event<OnStateChange, &StateSystem::on_state_change>(*this);
		service.register_event<OnStateActivate, &StateSystem::on_state_activate>(*this);

		// Commands:
		service.register_event<StateChangeCommand, &StateSystem::on_state_change_command>(*this);
		service.register_event<StateActivationCommand, &StateSystem::on_state_activation_command>(*this);

		return true;
	}

	bool StateSystem::on_unsubscribe(Service& service)
	{
		auto& registry = service.get_registry();

		registry.on_construct<StateComponent>().disconnect(*this);
		registry.on_update<StateComponent>().disconnect(*this);
		registry.on_destroy<StateComponent>().disconnect(*this);

		service.unregister(*this);

		return true;
	}

	Registry& StateSystem::get_registry() const
	{
		return get_service().get_registry();
	}

	const EntityDescriptor* StateSystem::get_descriptor(Entity entity) const
	{
		auto& registry = get_registry();

		const auto* instance_details = registry.try_get<InstanceComponent>(entity);

		if (!instance_details)
		{
			return nullptr;
		}

		return resource_manager.get_existing_descriptor(instance_details->instance);
	}

	std::optional<EntityStateIndex> StateSystem::get_state(Entity entity) const
	{
		auto& registry = get_registry();

		if (const auto* current_state = registry.try_get<StateComponent>(entity))
		{
			return current_state->state_index;
		}

		return std::nullopt;
	}

	bool StateSystem::set_state(Entity entity, std::string_view state_name) const
	{
		return set_state(entity, hash(state_name));
	}

	bool StateSystem::set_state(Entity entity, StringHash state_id) const
	{
		return set_state_by_id(entity, state_id);
	}

	bool StateSystem::set_state_by_id(Entity entity, StringHash state_id) const
	{
		const auto* descriptor = get_descriptor(entity);

		if (!descriptor)
		{
			return false;
		}

		const auto current_state = get_state(entity);

		const auto& from_state_index = current_state;
		
		const EntityState* from_state = (from_state_index)
			? descriptor->states.at(*from_state_index).get()
			: nullptr
		;

		const std::optional<StringHash> from_state_id = (from_state)
			? from_state->name
			: std::nullopt
		;

		const auto& to_state_id = state_id;

		const auto to_state = descriptor->get_state(to_state_id); // auto*

		if (!to_state)
		{
			return false;
		}

		const auto to_state_index = descriptor->get_state_index(to_state_id);

		//assert(to_state_index.has_value());

		if (!to_state_index)
		{
			return false;
		}

		auto new_state_info = EntityStateInfo { *to_state_index, to_state_id };

		auto old_state_info = (from_state)
			? EntityStateInfo { *from_state_index, from_state_id }
			: new_state_info
		;

		auto& registry = get_registry();

		if (to_state->has_activation_delay())
		{
			assert(from_state_index);

			// Decay the previous state, if present.
			if (from_state)
			{
				from_state->decay(registry, entity, *from_state_index, &to_state->components.persist);
			}

			// Force-update the state-component to reflect the new (unactivated) state.
			to_state->force_update_component(registry, entity, *to_state_index, from_state_index);
				
			// Notify listeners that we've changed state, but haven't activated yet.
			service->queue_event<OnStateChange>
			(
				entity,

				std::move(old_state_info),
				std::move(new_state_info),

				false // Delayed activation.
			);

			// Generate a timed event for activation, based on the delay indicated by the state:
			service->timed_event<StateActivationCommand>
			(
				*(to_state->activation_delay),

				entity,
				entity,

				to_state_id
			);

			return true;
		}

		if (descriptor->set_state_by_index(registry, entity, from_state_index, *to_state_index))
		{
			// Notify listeners that the state of `entity` has changed.
			service->queue_event<OnStateChange>
			(
				entity,

				std::move(old_state_info),
				new_state_info,

				true // Immediate activation.
			);

			// Notify listeners that activation has occurred.
			service->queue_event<OnStateActivate>
			(
				entity,

				std::move(new_state_info)
			);

			return true;
		}

		return false;
	}

	bool StateSystem::activate_state(Entity entity, StringHash state_id) const
	{
		const auto* descriptor = get_descriptor(entity);

		if (!descriptor)
		{
			return false;
		}

		const auto target_state_index = get_state(entity);

		if (!target_state_index)
		{
			return false;
		}

		const auto target_state = descriptor->states.at(*target_state_index).get();

		if (!target_state)
		{
			return false;
		}

		auto& registry = get_registry();

		target_state->activate(registry, entity, *target_state_index, std::nullopt, false);

		return true;
	}

	void StateSystem::on_update(const OnServiceUpdate& data)
	{
		// ...
	}

	void StateSystem::on_state_init(Registry& registry, Entity entity)
	{
		on_state_update_impl(registry, entity, false);
	}

	void StateSystem::on_state_update(Registry& registry, Entity entity)
	{
		on_state_update_impl(registry, entity);
	}

	void StateSystem::on_state_update_impl(Registry& registry, Entity entity, bool handle_existing)
	{
		const auto* descriptor = get_descriptor(entity);

		assert(descriptor);

		const auto& state_component = registry.get<StateComponent>(entity);

		if (handle_existing)
		{
			// No state-change occurred, no need to update anything.
			if (state_component.state_index == state_component.prev_state_index)
			{
				return;
			}

			for (auto& listener_entry : rule_listeners)
			{
				auto& listener = listener_entry.second;

				const auto prev_state_index = state_component.prev_state_index;
				const auto& prev_state = *descriptor->states[prev_state_index];

				listener.remove_rules(prev_state);
			}
		}

		const auto state_index = state_component.state_index;

		const auto& state = *descriptor->states[state_index];

		for (const auto& rule_entries : state.rules)
		{
			const auto& event_type_id = rule_entries.first;
			const auto& state_rules   = rule_entries.second;

			auto& listener = rule_listeners[event_type_id];

			if (!listener.is_active())
			{
				auto event_type = resolve(event_type_id);

				if (!event_type)
				{
					print_warn("Unable to resolve meta-type for event: #{}", event_type_id);

					continue;
				}

				auto connect_fn = event_type.func(hash("connect_meta_event"));

				if (!connect_fn)
				{
					print_warn("Unable to resolve connection function for meta event-listener -- type: #{}", event_type_id);

					continue;
				}

				auto& listener_ref = reinterpret_cast<MetaEventListener&>(listener);

				auto result = connect_fn.invoke({}, entt::forward_as_meta(listener_ref), entt::forward_as_meta(this->service));

				if (!result)
				{
					print_warn("Failed to invoke meta event-listener connection routine. -- type: #{}", event_type_id);
				}
			}

			listener.add_rules(state); // , event_type_id
		}
	}

	void StateSystem::on_state_destroyed(Registry& registry, Entity entity)
	{

	}
	
	// Handles logging/printing for debugging purposes.
	void StateSystem::on_state_change(const OnStateChange& state_change)
	{
		assert(state_change.to.id.has_value());

		if (state_change.from.id)
		{
			print("Entity {}: state changed from #{} to #{}", state_change.entity, *state_change.from.id, *state_change.to.id);
		}
		else
		{
			print("Entity {}: state changed to #{}", state_change.entity, *state_change.to.id);
		}
	}

	// Handles logging/printing for debugging purposes.
	void StateSystem::on_state_activate(const OnStateActivate& state_activate)
	{
		if (!state_activate.state.id)
		{
			return;
		}

		print("Entity {}: state #{} activated", state_activate.entity, *state_activate.state.id);
	}

	void StateSystem::on_state_change_command(const StateChangeCommand& state_change)
	{
		set_state(state_change.target, state_change.state_name);
	}

	void StateSystem::on_state_activation_command(const StateActivationCommand& state_activation)
	{
		auto result = activate_state(state_activation.target, state_activation.state_name);

		//assert(result);

		if (result)
		{
			if (state_activation.source == state_activation.target)
			{
				print("Entity {}: state #{} activated", state_activation.target, state_activation.state_name);
			}
			else
			{
				print("Entity {}: state #{} activated by Entity {}", state_activation.target, state_activation.state_name, state_activation.source);
			}
		}
		else
		{
			print_error("Failed to activate state #{} for Entity {} (activation by {})", state_activation.state_name, state_activation.target, state_activation.source);
		}
	}
}