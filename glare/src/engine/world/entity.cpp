#include "entity.hpp"

#include <engine/transform.hpp>

#include <engine/components/relationship_component.hpp>
#include <engine/components/type_component.hpp>

#include <engine/world/world.hpp>

#include <engine/events.hpp>
//#include "world_events.hpp"

namespace engine
{
	Entity create_entity(World& world, Entity parent, EntityType type)
	{
		auto& registry = world.get_registry();
		auto entity = registry.create();

		registry.emplace<TypeComponent>(entity, type);

		registry.emplace<TransformComponent>(entity);
		
		if (parent == null)
		{
			parent = world;
		}

		if (parent != null)
		{
			world.set_parent(entity, parent);

			/*
			auto rel = registry.get_or_emplace<RelationshipComponent>(entity); // RelationshipComponent();

			rel.set_parent(registry, entity, parent);

			registry.emplace_or_replace<RelationshipComponent>(entity, rel);
			*/

			/*
			auto parent_relationship = registry.get_or_emplace<RelationshipComponent>(parent);

			parent_relationship.add_child(registry, parent, entity);

			registry.replace<RelationshipComponent>(parent, [&](auto& r) { r = parent_relationship; });
			*/
		}

		world.queue_event<OnEntityCreated>(entity, parent, type);

		return entity;
	}

	// Currently no difference between a pivot and a default entity state.
	// (Other than the default value of the `type` field)
	Entity create_pivot(World& world, Entity parent, EntityType type)
	{
		return create_entity(world, parent, type);
	}

	Entity create_pivot(World& world, const math::Vector& position, Entity parent, EntityType type)
	{
		auto pivot = create_pivot(world, parent, type);

		auto t = world.get_transform(pivot);

		t.set_position(position);

		return pivot;
	}

	Entity create_pivot(World& world, const math::Vector& position, const math::Vector& rotation, const math::Vector& scale, Entity parent, EntityType type)
	{
		auto pivot = create_pivot(world, parent, type);

		auto t = world.get_transform(pivot);

		t.set_position(position);
		t.set_rotation(rotation);
		t.set_scale(scale);

		return pivot;
	}
}