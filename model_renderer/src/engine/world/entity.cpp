#include "entity.hpp"

#include <engine/relationship.hpp>
#include <engine/transform.hpp>
#include <engine/world/world.hpp>

namespace engine
{
	Entity create_entity(World& world, Entity parent)
	{
		auto& registry = world.get_registry();
		auto entity = registry.create();

		registry.assign<TransformComponent>(entity);
		
		if (parent == null)
		{
			parent = world;
		}

		if (parent != null)
		{
			auto parent_relationship = registry.get_or_assign<Relationship>(parent);

			parent_relationship.add_child(registry, parent, entity);

			registry.replace<Relationship>(parent, [&](auto& r) { r = parent_relationship; });
		}

		return entity;
	}

	void destory_entity(World& world, Entity entity)
	{
		world.get_registry().destroy(entity);
	}

	Entity create_pivot(World& world, Entity parent)
	{
		return create_entity(world, parent);
	}
}