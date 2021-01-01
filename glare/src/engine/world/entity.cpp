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
			world.set_parent(entity, parent);

			/*
			auto rel = registry.get_or_assign<Relationship>(entity); // Relationship();

			rel.set_parent(registry, entity, parent);

			registry.assign_or_replace<Relationship>(entity, rel);
			*/

			/*
			auto parent_relationship = registry.get_or_assign<Relationship>(parent);

			parent_relationship.add_child(registry, parent, entity);

			registry.replace<Relationship>(parent, [&](auto& r) { r = parent_relationship; });
			*/
		}

		return entity;
	}

	void destory_entity(World& world, Entity entity, bool destroy_orphans)
	{
		auto& registry = world.get_registry();

		auto* relationship = registry.try_get<Relationship>(entity);
		
		if (relationship != nullptr)
		{
			auto parent = relationship->get_parent();

			// Make the assumption this object exists, as if it didn't, we would be in an invalid state.
			auto parent_relationship = registry.get<Relationship>(parent);

			relationship->enumerate_child_entities(registry, [&](Entity child, Entity next_child) -> bool
			{
				if (destroy_orphans)
				{
					destory_entity(world, child, true);
				}
				else
				{
					parent_relationship.add_child(registry, parent, child);
				}

				return true;
			});

			registry.replace<Relationship>(parent, [&](auto& r) { r = parent_relationship; });
		}

		registry.destroy(entity);
	}

	Entity create_pivot(World& world, Entity parent)
	{
		return create_entity(world, parent);
	}

	Entity create_pivot(World& world, const math::Vector position, Entity parent)
	{
		auto pivot = create_pivot(world, parent);

		auto t = world.get_transform(pivot);

		t.set_position(position);

		return pivot;
	}
}