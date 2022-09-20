#include "motion.hpp"
#include "motion_component.hpp"

#include <engine/transform.hpp>

#include <engine/world/world.hpp>
#include <engine/world/physics/collision_component.hpp>
#include <engine/world/physics/physics_system.hpp>

namespace engine
{
	MotionSystem::MotionSystem(World& world, PhysicsSystem& physics)
		: WorldSystem(world), physics(physics)
	{
	}

	void MotionSystem::on_subscribe(World& world)
	{

	}

	void MotionSystem::on_update(World& world, float delta)
	{
		auto& registry = world.get_registry();

		auto view = registry.view<MotionComponent, CollisionComponent>(); // <TransformComponent, ...> (auto& tf_comp)

		// Apply motion (gravity, velocity, deceleration, etc.) and resolve collisions between old and new positions:
		view.each([&](auto entity, auto& motion, auto& collision)
		{
			auto transform = world.get_transform(entity);

			update_motion(entity, transform, motion, collision, delta);
		});
	}

	void MotionSystem::update_motion
	(
		Entity entity,
		Transform& transform,
		MotionComponent& motion,
		CollisionComponent& collision,
		float delta
	)
	{
		math::Vector movement = {};

		if (motion.apply_gravity)
		{
			auto gravity = (get_gravity() * delta);

			if (!motion.on_ground)
			{
				movement += gravity;
			}

			// Check gravity:
			auto when_gravity_applied = (transform.get_position() + gravity);

			auto ground = physics.cast_to(collision, when_gravity_applied);

			if (ground)
			{
				if (!motion.on_ground)
				{
					motion.ground = ground->hit_entity;

					print("Hit the ground.");


					//world.event<OnAirToGround>(entity, );

					motion.on_ground = true;
				}
			}
			else
			{
				if (motion.on_ground)
				{
					print("Off ground");

					motion.on_ground = false;
				}
			}

			//cast_to(...)
		}

		if (motion.apply_velocity)
		{
			movement += (motion.velocity * delta);
		}

		auto decel = std::abs(motion.ground_deceleration);

		if (decel > 0.0f)
		{
			if (glm::length(motion.velocity) > MIN_SPEED)
			{
				motion.velocity -= (motion.velocity * decel * delta); // math::Vector
			}
			else
			{
				motion.velocity = {};
			}
		}

		transform.move(movement);
	}

	// Internal shorthand for `world.get_gravity()`.
	math::Vector MotionSystem::get_gravity() const
	{
		return world.get_gravity();
	}

	// Internal shorthand for `world.down()`.
	math::Vector MotionSystem::down() const
	{
		return world.down();
	}
}