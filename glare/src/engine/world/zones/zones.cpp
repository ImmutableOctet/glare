#include "zones.hpp"

#include <engine/resource_manager/resource_manager.hpp>
#include <engine/world/world.hpp>
#include <engine/world/entity.hpp>
#include <engine/world/physics/collision_config.hpp>
#include <engine/world/physics/components/collision_component.hpp>

namespace engine
{
	ZoneSystem::ZoneSystem(World& world)
		: WorldSystem(world)
	{}

	void ZoneSystem::on_subscribe(World& world)
	{

	}

	void ZoneSystem::on_update(World& world, float delta)
	{

	}

	Entity create_zone(World& world, const math::AABB& bounds, EntityType type, Entity parent, bool update_transform)
	{
		auto entity = create_zone(world, bounds.dim_lengths(), type, parent);

		if (update_transform)
		{
			auto tform = world.get_transform(entity);

			auto center_point = bounds.get_center_point();

			tform.set_position(center_point);
		}

		return entity;
	}

	Entity create_zone(World& world, const math::Vector extents, EntityType type, Entity parent)
	{
		auto& resources = world.get_resource_manager();
		auto& registry = world.get_registry();

		auto entity = create_entity(world, parent, type);
		auto half_extents = (extents / 2.0f);

		//auto collision_data = resources.generate_cube_collision(half_extents);
		auto collision_data = resources.generate_sphere_collision(glm::length(half_extents));

		assert(collision_data.has_shape());

		attach_collision(world, entity, collision_data.collision_shape, CollisionConfig(type));

		//registry.emplace_or_replace<>(entity);

		return entity;
	}
}