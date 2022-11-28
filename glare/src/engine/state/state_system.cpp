#include "state_system.hpp"
#include "events.hpp"

#include "components/state_component.hpp"

#include <engine/components/instance_component.hpp>

#include <engine/service.hpp>
#include <engine/service_events.hpp>
#include <engine/meta/meta.hpp>
#include <engine/entity_descriptor.hpp>
#include <engine/entity_state.hpp>

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

		registry.on_construct<StateComponent>().connect<&StateSystem::on_state_init>(*this);
		registry.on_update<StateComponent>().connect<&StateSystem::on_state_update>(*this);
		registry.on_destroy<StateComponent>().connect<&StateSystem::on_state_destroyed>(*this);

		service.register_event<OnServiceUpdate, &StateSystem::on_update>(*this);

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
		return set_state_by_id(entity, hash(state_name));
	}

	bool StateSystem::set_state_by_id(Entity entity, StringHash state_id) const
	{
		auto& registry = get_registry();
		const auto* descriptor = get_descriptor(entity);

		if (!descriptor)
		{
			return false;
		}

		auto current_state = get_state(entity);

		if (descriptor->set_state_by_id(registry, entity, current_state, state_id))
		{
			const auto& from_state_index = *current_state;
			const auto& from_state_id = descriptor->states[from_state_index]->name;

			const auto new_state = get_state(entity);

			assert(new_state);

			const auto& to_state_index = *new_state;
			const auto& to_state_id = state_id;

			service->queue_event<OnStateChange>
			(
				entity,

				EntityStateInfo{ from_state_index, from_state_id },
				EntityStateInfo{ to_state_index, to_state_id }
			);

			return true;
		}


		return false;
	}

	bool StateSystem::set_state_by_index(Entity entity, EntityStateIndex state_index) const
	{
		auto& registry = get_registry();
		const auto* descriptor = get_descriptor(entity);

		if (!descriptor)
		{
			return false;
		}

		auto current_state = get_state(entity);

		if (descriptor->set_state_by_index(registry, entity, current_state, state_index))
		{
			const auto& from_state_index = *current_state;
			const auto& from_state_id = descriptor->states[from_state_index]->name;

			const auto& to_state_index = state_index;
			const auto& to_state_id = descriptor->states[to_state_index]->name;

			service->queue_event<OnStateChange>
			(
				entity,

				EntityStateInfo { from_state_index, from_state_id },
				EntityStateInfo { to_state_index, to_state_id }
			);

			return true;
		}

		return false;
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
}