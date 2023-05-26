#include "delta_system.hpp"

#include <engine/transform.hpp>
#include <engine/world/world_events.hpp>

#include <engine/components/transform_component.hpp>
#include <engine/components/transform_history_component.hpp>

#include <engine/world/world.hpp>

namespace engine
{
	DeltaSystem::DeltaSystem(World& world, DeltaTime::Rate ideal_rate) :
		WorldSystem(world),
		delta_time(ideal_rate)
	{
		world.subscribe(*this);
	}

	const DeltaTime& DeltaSystem::update_delta(app::Milliseconds time)
	{
		// Update the delta-timer.
		delta_time << time;

		return delta_time;
	}

	float DeltaSystem::get_delta() const
	{
		return delta_time.get_delta();
	}

	void DeltaSystem::on_subscribe(World& world)
	{
		//auto& registry = get_registry();
		
		world.register_event<OnTransformChanged, &DeltaSystem::on_transform_changed>(*this);
	}

	void DeltaSystem::on_unsubscribe(World& world)
	{
		//auto& registry = get_registry();

		// ...

		world.unregister(*this);
	}

	void DeltaSystem::on_update(World& world, float delta)
	{
		// ...
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
}