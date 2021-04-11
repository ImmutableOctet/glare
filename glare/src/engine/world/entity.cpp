#include "entity.hpp"

#include <engine/relationship.hpp>
#include <engine/transform.hpp>
#include <engine/world/world.hpp>
#include <engine/type_component.hpp>

#include <engine/events/events.hpp>

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
			auto rel = registry.get_or_emplace<Relationship>(entity); // Relationship();

			rel.set_parent(registry, entity, parent);

			registry.emplace_or_replace<Relationship>(entity, rel);
			*/

			/*
			auto parent_relationship = registry.get_or_emplace<Relationship>(parent);

			parent_relationship.add_child(registry, parent, entity);

			registry.replace<Relationship>(parent, [&](auto& r) { r = parent_relationship; });
			*/
		}

		world.queue_event<OnEntityCreated>(entity, parent, type);

		return entity;
	}

	void destory_entity(World& world, Entity entity, bool destroy_orphans)
	{
		auto& registry = world.get_registry();

		auto* relationship = registry.try_get<Relationship>(entity);

		auto root = world.get_root();
		
		Entity parent = ((relationship) ? relationship->get_parent() : null);

		EntityType type = EntityType::Other; // EntityType::Default;

		auto* tp = registry.try_get<TypeComponent>(entity);

		if (tp)
		{
			type = tp->type;
		}

		// Actual destruction takes place once the 'World' object has received the event.
		world.queue_event<OnEntityDestroyed>(entity, parent, type, destroy_orphans);
	}

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