#include "delta_system.hpp"

#include "events.hpp"

#include "components/delta_component.hpp"
#include "components/lifetime_delta_component.hpp"
#include "components/delta_tracker_component.hpp"

#include <engine/transform.hpp>
#include <engine/world/world_events.hpp>

#include <engine/components/transform_component.hpp>
#include <engine/components/transform_history_component.hpp>

#include <engine/entity/entity_system.hpp>

#include <engine/meta/runtime_traits.hpp>
#include <engine/meta/events.hpp>

#include <engine/world/world.hpp>

#include <cassert>

namespace engine
{
	DeltaSystem::DeltaSystem
	(
		World& world,

		EntitySystem& entity_system,

		DeltaTime::TicksPerSecond ideal_rate,
		Mode mode,
		bool only_track_tagged_entities
	) :
		WorldSystem(world),
		entity_system(entity_system),
		delta_time(ideal_rate),
		mode(mode),
		only_track_tagged_entities(only_track_tagged_entities)
	{
		world.subscribe(*this);
	}

	void DeltaSystem::snapshot(DeltaTimestamp timestamp)
	{
		auto& service = get_world();
		auto& registry = get_registry();

		service.event<OnDeltaSnapshot>
		(
			timestamp,
			get_latest_delta_timestamp(),
			get_delta()
		);

		registry.view<DeltaComponent>().each
		(
			[](Entity entity, DeltaComponent& delta_comp)
			{
				delta_comp.clear();
			}
		);

		registry.view<LifetimeDeltaComponent>().each
		(
			[](Entity entity, LifetimeDeltaComponent& lifetime_delta_comp)
			{
				lifetime_delta_comp.clear();
			}
		);
	}

	const DeltaTime& DeltaSystem::update_delta(TimePoint time)
	{
		// Update the delta-timer.
		delta_time << time;

		return delta_time;
	}

	float DeltaSystem::get_delta() const
	{
		return delta_time.get_delta();
	}

	DeltaTimestamp DeltaSystem::get_latest_delta_timestamp() const
	{
		return static_cast<DeltaTimestamp>(delta_time.current_frame_time().time_since_epoch().count());
	}

	void DeltaSystem::on_subscribe(World& world)
	{
		auto& service = world;
		
		auto& registry = world.get_registry(); // get_registry();
		
		registry.on_construct<DeltaTrackerComponent>().connect<&DeltaSystem::on_delta_tracker_create>(*this);
		registry.on_destroy<DeltaTrackerComponent>().connect<&DeltaSystem::on_delta_tracker_destroy>(*this);
		registry.on_update<DeltaTrackerComponent>().connect<&DeltaSystem::on_delta_tracker_update>(*this);
		
		service.register_event<OnTransformChanged, &DeltaSystem::on_transform_changed>(*this);

		service.register_event<OnComponentCreate, &DeltaSystem::on_component_create>(*this);
		service.register_event<OnComponentDestroy, &DeltaSystem::on_component_destroy>(*this);
		service.register_event<OnComponentUpdate, &DeltaSystem::on_component_update>(*this);

		listen
		(
			(!only_track_tagged_entities)
		);
	}

	void DeltaSystem::on_unsubscribe(World& world)
	{
		//auto& registry = get_registry();

		// ...

		world.unregister(*this);
	}

	std::size_t DeltaSystem::listen_for_components()
	{
		std::size_t components_identified = 0;

		for (const auto& type_entry : entt::resolve())
		{
			const auto& type = type_entry.second;

			if ((type_is_component(type)) && (!type_is_history_component(type)))
			{
				if (entity_system.listen(type))
				{
					components_identified++;
				}
			}
		}

		return components_identified;
	}

	void DeltaSystem::listen(bool components)
	{
		if (components)
		{
			// TODO: Revisit concept of detecting which components should be listened for.
			listen_for_components();
		}
	}

	EntityListener* DeltaSystem::listen(const MetaType& component_type)
	{
		if (!component_type)
		{
			return {};
		}

		return entity_system.listen(component_type);
	}

	EntityListener* DeltaSystem::listen(MetaTypeID component_type_id)
	{
		return listen(resolve(component_type_id));
	}

	void DeltaSystem::on_delta_tracker_listen(Entity entity, const DeltaTrackerComponent& tracker)
	{
		for (const auto& component_type_id : tracker.tracked_components)
		{
			const auto component_type = resolve(component_type_id);

			listen(component_type); // entity, ...
		}
	}

	void DeltaSystem::on_delta_tracker_listen(Registry& registry, Entity entity)
	{
		const auto& tracker = registry.get<DeltaTrackerComponent>(entity);

		on_delta_tracker_listen(entity, tracker);
	}

	void DeltaSystem::on_delta_tracker_create(Registry& registry, Entity entity)
	{
		on_delta_tracker_listen(registry, entity);
	}

	void DeltaSystem::on_delta_tracker_destroy(Registry& registry, Entity entity)
	{
		// TODO: Implement some form of 'stop listening' logic:
		/*
		const auto& tracker = registry.get<DeltaTrackerComponent>(entity);

		for (const auto& component_type_id : tracker.tracked_components)
		{
			const auto component_type = resolve(component_type_id);

			stop_listening(entity, component_type);
		}
		*/
	}

	void DeltaSystem::on_delta_tracker_update(Registry& registry, Entity entity)
	{
		on_delta_tracker_listen(registry, entity);
	}

	void DeltaSystem::on_update(World& world, float delta)
	{
		// ...
	}

	void DeltaSystem::on_fixed_update(World& world, TimePoint time)
	{
		if (mode != Mode::FixedUpdate)
		{
			return;
		}

		snapshot(static_cast<DeltaTimestamp>(time.time_since_epoch().count()));
	}

	void DeltaSystem::on_transform_changed(const OnTransformChanged& tform_change)
	{
		auto& registry = get_registry();

		const auto entity = tform_change.entity;

		auto* tform_history = registry.try_get<TransformHistoryComponent>(entity);

		if (tform_history)
		{
			auto tform = world.get_transform(entity);

			*tform_history << tform;
		}

		// Debugging related:
		/*
		auto* type_component = registry.try_get<TypeComponent>(entity);
		auto entity_type = ((type_component) ? type_component->type : EntityType::Default);

		std::string name_label;

		auto* name_component = registry.try_get<NameComponent>(entity);

		if (name_component)
		{
			name_label = " \"" + name_component->get_name() +"\"";
		}

		print("Entity #{}{} ({}) - Transform Changed: {}", entity, name_label, entity_type, get_transform(entity).get_vectors());
		*/
	}

	void DeltaSystem::on_component_create(const OnComponentCreate& component_details)
	{
		assert(component_details.component);
		
		const auto component_type = component_details.component.type();

		if (!component_type)
		{
			return;
		}

		const auto component_id = component_type.id();

		assert(component_id);

		const auto& entity = component_details.entity;

		if (!is_tracked_component(entity, component_id))
		{
			return;
		}
		
		auto& service = get_world();
		auto& registry = get_registry();

		registry.patch<LifetimeDeltaComponent>
		(
			entity,

			[this, &service, entity, &component_details, component_id](LifetimeDeltaComponent& delta_comp)
			{
				if (delta_comp.add_created(component_id))
				{
					service.event<OnDeltaLifetimeBegin>
					(
						entity, component_id, &delta_comp
					);
				}
				else
				{
					service.event<OnDeltaLifetimeRestart>
					(
						entity, component_id, &delta_comp
					);

					/*
						Marking the component as 'created' has failed.
						This is generally the result of a 'destruction' entry already being present.
						
						The presence of a 'destruction' followed by a 'creation' allows
						us to infer that the component existed during the last 'snapshot';

						we can then assume that the intended operation is actually a 'modification'.
					*/
					on_component_update
					(
						OnComponentUpdate
						{
							component_details.entity,
							component_details.component.as_ref()
						}
					);
				}
			}
		);
	}

	void DeltaSystem::on_component_destroy(const OnComponentDestroy& component_details)
	{
		assert(component_details.component);
		
		const auto component_type = component_details.component.type();

		if (!component_type)
		{
			return;
		}

		const auto component_id = component_type.id();

		assert(component_id);

		const auto& entity = component_details.entity;

		if (!is_tracked_component(entity, component_id))
		{
			return;
		}

		auto& service = get_world();
		auto& registry = get_registry();

		registry.patch<LifetimeDeltaComponent>
		(
			entity,

			[&service, entity, component_id](LifetimeDeltaComponent& delta_comp)
			{
				delta_comp.add_destroyed(component_id);

				service.event<OnDeltaLifetimeEnd>
				(
					entity, component_id, &delta_comp
				);
			}
		);
	}

	void DeltaSystem::on_component_update(const OnComponentUpdate& component_details)
	{
		assert(component_details.component);
		
		const auto component_type = component_details.component.type();

		if (!component_type)
		{
			return;
		}

		const auto component_id = component_type.id();

		assert(component_id);

		const auto& entity = component_details.entity;

		if (!is_tracked_component(entity, component_id))
		{
			return;
		}

		auto& registry = get_registry();

		registry.patch<DeltaComponent>
		(
			entity,

			[component_id](DeltaComponent& delta_comp)
			{
				delta_comp.add_modified(component_id);
			}
		);
	}

	bool DeltaSystem::is_tracked_component(Entity entity, MetaTypeID component_type_id) const
	{
		const auto& registry = get_registry();

		if (const auto tracker_comp = registry.try_get<DeltaTrackerComponent>(entity))
		{
			return tracker_comp->tracked_components.includes(component_type_id);
		}

		// Only track this entity's component if we're not restricted.
		return (!only_track_tagged_entities);
	}
}