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
		//auto& registry = service.get_registry();

		// Meta:
		service.register_event<OnServiceUpdate, &StateSystem::on_update>(*this);

		return true;
	}

	Registry& StateSystem::get_registry() const
	{
		return get_service().get_registry();
	}

	const EntityDescriptor* StateSystem::get_descriptor(Entity entity) const
	{
		auto& registry = get_registry();

		auto* instance_details = registry.try_get<InstanceComponent>(entity);

		if (!instance_details)
		{
			return nullptr;
		}
		
		const auto* factory = resource_manager.get_existing_factory(instance_details->instance);

		const auto& descriptor = factory->get_descriptor();

		return &descriptor;
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
			const auto& from_state_id = descriptor->states[from_state_index].name;

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
			const auto& from_state_id = descriptor->states[from_state_index].name;

			const auto& to_state_index = state_index;
			const auto& to_state_id = descriptor->states[to_state_index].name;

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
}