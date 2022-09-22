#include "motion_system.hpp"

#include "motion_component.hpp"
#include "motion_events.hpp"
#include "motion_attachment_proxy.hpp"
#include "ground.hpp"

#include <engine/transform.hpp>
#include <engine/relationship.hpp>

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
		world.register_event<OnAirToGround,    &MotionSystem::on_air_to_ground>(*this);
		world.register_event<OnGroundToAir,    &MotionSystem::on_ground_to_air>(*this);
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

		auto entity = world.get_forwarded(surface.collision.a);

		auto* motion = registry.try_get<MotionComponent>(entity);

		if (!motion)
		{
			return;
		}

		auto floor_detection = surface.floor();

		print("FLOOR: {}", floor_detection);

		// TODO: Add surface dot-product handling so we don't treat everything we touch like it's ground.
		if (floor_detection < GROUND)
		{
			// The slope we're contacting is too steep to be considered ground.
			return;
		}

		print("Ground surface detected.");

		motion->ground = surface;
		motion->ground.update_metadata(world);

		if (!motion->on_ground)
		{
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

	Entity MotionSystem::detach_motion_proxy(Entity entity, MotionComponent& motion, Entity _opt_new_proxy)
	{
		print("Detached.");

		auto& registry = world.get_registry();
		auto& attachment_proxy = registry.get_or_emplace<MotionAttachmentProxy>(entity, null);

		// The 'proxy' entity, representing the object we're attached to.
		Entity proxy = null;

		if (attachment_proxy.is_active()) // Acts as a null-check.
		{
			// Retrieve the current proxy entity.
			proxy = world.get_parent(entity);

			// Safety/efficiency check; if we're attaching to the same object, don't do anything.
			if ((_opt_new_proxy != null) && (_opt_new_proxy == proxy))
			{
				return proxy;
			}

			// Restore the old/intended parent entity.
			world.set_parent(entity, attachment_proxy.intended_parent);

			// Set the intended parent back to `null`, changing `attachment_proxy` to 'inactive' by extension.
			attachment_proxy.intended_parent = null;
			
			// Notify listeners that `entity` is no longer attached to `proxy`.
			world.queue_event<OnMotionDetachment>(entity, proxy);
		}

		// Update our internal state to no longer reflect attachment.
		motion.is_attached = false;

		// Return the detached 'proxy' entity, if one exists.
		return proxy;
	}

	Entity MotionSystem::attach_motion_proxy(Entity entity, MotionComponent& motion, Entity proxy)
	{
		print("Attached.");

		// Check if we're already attached to an object:
		if (motion.is_attached)
		{
			// Detach from the current object.
			auto response = detach_motion_proxy(entity, motion, proxy);

			// If the response from `detach_motion_proxy` is the same as `proxy`,
			// we're already attached to this object, meaning we can return immediately.
			if (response == proxy)
			{
				return null;
			}
		}

		auto& registry = world.get_registry();

		// Retrieve the specified `entity`'s current/intended parent.
		auto intended_parent = world.get_parent(entity);

		// Store the intended parent via `MotionAttachmentProxy`.
		auto& attachment_proxy = registry.emplace_or_replace<MotionAttachmentProxy>(entity, intended_parent);

		// Update `entity`'s parent to the specified `proxy` entity.
		world.set_parent(entity, proxy);

		// Notify listeners that we attached to `proxy`.
		world.queue_event<OnMotionAttachment>(entity, proxy);

		// Update our internal state to reflect attachment.
		motion.is_attached = true;

		// Return the intended (original) parent to the caller.
		return intended_parent;
	}

	void MotionSystem::on_air_to_ground(const OnAirToGround& to_ground)
	{
		return;
		auto& registry = world.get_registry();

		auto entity = to_ground.entity();
		auto& motion = registry.get<MotionComponent>(entity);

		auto ground = to_ground.ground();

		// In the event this is the same ground entity we're currently attached to, return immediately. (Sanity check)
		if ((ground == motion.ground) && (motion.on_ground))
		{
			return;
		}

		if ((motion.attach_to_dynamic_ground) && to_ground.is_dynamic_ground())
		{
			attach_motion_proxy(entity, motion, ground);
		}
	}

	void MotionSystem::on_ground_to_air(const OnGroundToAir& to_air)
	{
		return;
		auto& registry = world.get_registry();
		
		auto entity = to_air.entity();
		auto& motion = registry.get<MotionComponent>(entity);

		if (to_air.was_dynamic_ground() && (motion.is_attached))
		{
			detach_motion_proxy(entity, motion);
		}
	}

	std::optional<CollisionCastResult> MotionSystem::detect_air
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

		if (ground_check)
		{
			// INSERT GROUND ALIGNMENT LOGIC HERE. (or in `on_surface_contact`...? - not sure yet)
		}
		else
		{
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

		auto orientation = transform.get_basis(); // get_local_basis();

		math::Vector gravity;

		if (motion.apply_gravity)
		{
			gravity = (get_gravity() * delta);

			// NOTE: We apply gravity this update, even if `detect_ground` detects a hit.
			if (!motion.on_ground)
			{
				movement += gravity;
				//velocity += gravity;
			}
		}

		// Align the intended movement to the rotation of the entity.
		movement = transform.align_vector(movement);

		// Handle ground/air detection:
		if (motion.apply_gravity)
		{
			// Before moving this entity, look for ground collision.
			// 
			// NOTE: Since gravity is applied based on the entity's local rotation,
			// we should align `movement` before calling this function.
			detect_air(data, transform.align_vector(gravity), movement);
		}

		// NOTE: Since we call `align_vector` before moving the entity,
		// we don't need to use local (i.e. aligned) movement.
		transform.move(movement, false);
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