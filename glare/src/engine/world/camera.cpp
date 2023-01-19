#include "camera.hpp"

#include <engine/world/entity.hpp>
#include <engine/world/world.hpp>

// NOTE: May remove/rework this funcitonality later.
#include <engine/world/physics/components/collision_component.hpp>

#include <engine/resource_manager/resource_manager.hpp>

namespace engine
{
	Entity create_camera(World& world, CameraComponent params, Entity parent, bool make_active, bool collision_enabled)
	{
		auto& registry = world.get_registry();
		
		constexpr auto entity_type = EntityType::Camera;
		auto entity = create_entity(world, parent, entity_type);

		registry.emplace<CameraComponent>(entity, params);

		// Assign a default name for this camera.
		world.set_name(entity, "Camera");

		// TODO: Remove first-class collision support.
		if (collision_enabled)
		{
			auto& resource_manager = world.get_resource_manager();
			
			auto collision_data = resource_manager.generate_sphere_collision(2.0f); // 0.1f
			//auto collision_data = resource_manager.generate_capsule_collision(1.0f, 2.0f);

			auto collision_config = CollisionConfig { entity_type };
			//auto collision_config = CollisionConfig{ EntityType::Platform };

			//collision_config.group = CollisionGroup::DynamicGeometry;
			
			float mass = 0.0f;

			attach_collision(world, entity, collision_data.collision_shape, collision_config, mass);
		}
		
		if ((world.get_camera() == null) || make_active)
		{
			world.set_camera(entity);
		}

		return entity;
	}
}