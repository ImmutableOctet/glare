#include "motion_system.hpp"

#include "motion_component.hpp"
#include "motion_events.hpp"
#include "ground.hpp"

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
		world.register_event<OnSurfaceContact, &MotionSystem::on_surface_contact>(*this);
	}

	void MotionSystem::on_update(World& world, float delta)
	{
		auto& registry = world.get_registry();

		auto view = registry.view<MotionComponent, CollisionComponent>(); // <TransformComponent, ...> (auto& tf_comp)

		// Apply motion (gravity, velocity, deceleration, etc.) and resolve collisions between old and new positions:
		view.each([&](auto entity, auto& motion, auto& collision)
		{
			auto transform = world.get_transform(entity);

			update_motion({ entity, transform, motion, collision, delta });
		});
	}

	void MotionSystem::on_surface_contact(const OnSurfaceContact& surface)
	{
		auto& registry = world.get_registry();

		auto& entity = surface.collision.a;

		auto* motion = registry.try_get<MotionComponent>(entity);

		if (!motion)
		{
			return;
		}

		print("Surface detected.");

		if (!motion->on_ground)
		{
			motion->ground = surface;
			motion->ground.update_metadata(world);

			print("Hit the ground.");

			auto& ground_position = surface.collision.position;

			auto landing_vector = surface.impact_velocity;

			///*
			world.event<OnAirToGround>
			(
				//entity,
				motion->ground
			);
			//*/

			motion->on_ground = true;
		}
	}

	std::optional<CollisionCastResult>
	MotionSystem::handle_ground_to_air
	(
		const EntityData& data,
		const math::Vector& gravity_movement,
		const math::Vector& projected_movement
	)
	{
		auto& transform = data.transform;
		auto& motion = data.motion;
		auto& collision = data.collision;

		if (!motion.on_ground)
		{
			// Already in the air.
			return std::nullopt;
		}

		auto entity_position = transform.get_position();
		auto if_gravity_applied = (entity_position + gravity_movement);
		auto ground_check = physics.cast_to(collision, if_gravity_applied);

		if (!ground_check)
		{
			print("Off ground");

			// NOTE: The reverse scenario, `OnAirToGround` occurs in the `on_surface_contact` routine.
			world.event<OnGroundToAir>
			(
				//data.entity,
				motion.ground,

				entity_position,
				projected_movement
			);

			motion.on_ground = false;
		}

		return ground_check;
	}

	void MotionSystem::update_motion(const EntityData& data)
	{
		auto& transform = data.transform;
		auto& motion = data.motion;
		auto& collision = data.collision;
		auto& delta = data.delta;

		math::Vector movement = {};

		if (motion.apply_velocity)
		{
			movement += (motion.velocity * delta);
		}

		if (motion.on_ground)
		{
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
		}

		if (motion.apply_gravity)
		{
			auto gravity = (get_gravity() * delta);

			// NOTE: We apply gravity this update, even if `detect_ground` detects a hit.
			if (!motion.on_ground)
			{
				movement += gravity;
				//velocity += gravity;
			}

			// Before moving this entity, look for ground collision.
			handle_ground_to_air(data, gravity, movement);
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