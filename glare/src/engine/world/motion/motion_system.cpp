#include "motion_system.hpp"

#include "motion_events.hpp"

#include "components/motion_component.hpp"
#include "components/velocity_component.hpp"
#include "components/ground_component.hpp"
#include "components/gravity_component.hpp"
#include "components/alignment_proxy_component.hpp"
#include "components/motion_attachment_proxy_component.hpp"
#include "components/acceleration_component.hpp"
#include "components/deceleration_component.hpp"
#include "components/directional_influence_component.hpp"
#include "components/focus_component.hpp"
#include "components/direction_component.hpp"
#include "components/orientation_component.hpp"
#include "components/rotate_component.hpp"

#include <math/types.hpp>
#include <math/bullet.hpp>
#include <math/comparison.hpp>

#include <engine/transform.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/components/relationship_component.hpp>
//#include <engine/components/transform_history_component.hpp>

#include <engine/world/world.hpp>
#include <engine/world/physics/ground.hpp>
#include <engine/world/physics/physics_system.hpp>
#include <engine/world/physics/components/collision_component.hpp>

#include <algorithm>

namespace engine
{
	// Utility function that enumerates entities with a fully qualified `Transform` object.
	template <typename ...ComponentTypes, typename Callable, typename ...Exclude>
	static void update_transform(Registry& registry, Callable callback, entt::exclude_t<Exclude>... exclude)
	{
		registry.view<RelationshipComponent, TransformComponent, ComponentTypes...>(exclude...).each([&](auto entity, auto& rel_comp, auto& tform_comp, ComponentTypes&... comps)
		{
			auto transform = Transform(registry, entity, rel_comp, tform_comp);

			callback(entity, transform, comps...);
		});
	}

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
		apply_velocity(delta);
		update_gravity(delta);
		handle_deceleration(delta);
		update_focus(delta);
		update_turning_objects(delta);
	}

	void MotionSystem::apply_velocity(float delta)
	{
		auto& registry = get_registry();

		update_transform<VelocityComponent>
		(
			registry,

			[delta](Entity entity, Transform& transform, VelocityComponent& vel_comp)
			{
				transform.move((vel_comp.velocity * delta), false);
			}
		);
	}

	void MotionSystem::update_gravity(float delta)
	{
		auto& registry = get_registry();

		auto apply = [this, delta](Entity entity, Transform& transform, const GravityComponent& gravity_comp)
		{
			// TODO: Look into reworking gravity to apply to velocity instead.
			auto gravity = gravity_comp.get_vector(get_gravity(), delta);

			transform.move(gravity, false); // true
		};

		update_transform<GroundComponent, CollisionComponent, GravityComponent>
		(
			registry,

			[this, &apply, delta](Entity entity, Transform& transform, GroundComponent& ground, const CollisionComponent& collision, const GravityComponent& gravity)
			{
				auto ground_check = detect_air(entity, transform, ground, collision, gravity, delta);

				if (!ground_check) // if (!ground_comp.on_ground())
				{
					apply(entity, transform, gravity);
				}
			}
		);

		// Gravity component, no ground-detection:
		update_transform<GravityComponent>(registry, apply, entt::exclude<GroundComponent>);
	}

	void MotionSystem::handle_deceleration(float delta)
	{
		auto& registry = get_registry();

		update_transform<VelocityComponent, DecelerationComponent>
		(
			get_registry(),

			[&registry, delta](Entity entity, Transform& transform, VelocityComponent& vel_comp, DecelerationComponent& decel_comp)
			{
				float ground_traction = 1.0f;

				bool on_ground = true;

				if (auto ground_comp = registry.try_get<GroundComponent>(entity))
				{
					on_ground = ground_comp->on_ground();

					/*
					if (on_ground)
					{
						ground_traction = ...;
					}
					*/
				}

				const auto speed = vel_comp.speed();

				// TODO: Look into replacing statically defined minimum-speed value.
				if (speed > MIN_SPEED) // 0.025f
				{
					auto deceleration = (on_ground)
						? decel_comp.ground[speed]
						: decel_comp.air[speed]
					;

					//vel_comp.velocity -= ((vel_comp.velocity * deceleration) * delta);

					vel_comp.velocity = math::lerp(vel_comp.velocity, math::Vector { 0.0f, 0.0f, 0.0f }, (deceleration * delta));
				}
				else
				{
					vel_comp.velocity = {};
				}

				// Mark `VelocityComponent` as patched.
				registry.patch<VelocityComponent>(entity);
			}
		);
	}

	void MotionSystem::update_focus(float delta)
	{
		auto& registry = get_registry();

		update_transform<FocusComponent>
		(
			registry,

			[delta, &registry](Entity entity, Transform& entity_transform, const FocusComponent& focus)
			{
				const auto& target = focus.target;

				if (target == null)
				{
					return;
				}

				const auto& focus_offset = focus.focus_offset;

				auto target_tform = Transform(registry, target);

				const auto target_position = (target_tform.get_position() + focus_offset);

				entity_transform.look_at(target_position, (focus.tracking_speed * delta));

				/*
				// Alternative implementation:
				const auto focus_direction = glm::normalize(target_position - entity_position);

				entity_transform.set_direction_vector(focus_direction, (focus.tracking_speed * delta));
				*/
			}
		);
	}

	void MotionSystem::update_turning_objects(float delta)
	{
		auto& registry = get_registry();

		update_transform<DirectionComponent>
		(
			registry,

			[delta, &registry](Entity entity, Transform& entity_transform, const DirectionComponent& turn)
			{
				entity_transform.set_direction_vector
				(
					turn.direction,
					(turn.turn_speed * delta),
					(!turn.ignore_x), (!turn.ignore_y), (!turn.ignore_z)
				);
			}
		);

		update_transform<OrientationComponent>
		(
			registry,

			[delta, &registry](Entity entity, Transform& entity_transform, const OrientationComponent& orientation_comp)
			{
				entity_transform.set_basis_q(orientation_comp.orientation, (orientation_comp.turn_speed * delta));
			}
		);

		update_transform<RotateComponent>
		(
			registry,

			[delta, &registry](Entity entity, Transform& entity_transform, const RotateComponent& rotate_comp)
			{
				entity_transform.apply_basis_q(rotate_comp.relative_orientation, (rotate_comp.turn_speed * delta));

				// Alternative implementation:
				//entity_transform.set_basis_q(rotate_comp.get_next_basis(entity_transform, delta));
			}
		);
	}

	void MotionSystem::on_surface_contact(const OnSurfaceContact& surface)
	{
		auto& registry = world.get_registry();

		auto entity = world.get_forwarded(surface.collision.a);

		auto* ground = registry.try_get<GroundComponent>(entity);

		if (!ground)
		{
			return;
		}

		const auto floor_detection = surface.floor();

		if (floor_detection < ground->get_detection_threshold()) // GROUND
		{
			// The slope we're contacting is too steep to be considered ground.
			return;
		}

		// Debugging related:
		if ((int)entity == 14)
		{
			print("Ground surface detected.");
			print("surface.slope(): {}", surface.slope());
			print("Forward: {}", surface.forward());
		}

		auto* motion = registry.try_get<MotionComponent>(entity);

		if (!ground->on_ground())
		{
			ground->ground = surface;
			//ground->set_on_ground(true); // <-- Should already happen by default.
			ground->ground.update_metadata(world);

			print("Hit the ground.");

			auto& ground_position = surface.collision.position;

			auto landing_vector = surface.impact_velocity;

			world.event<OnAirToGround>
			(
				//entity, // <-- Already handled through the ground/surface object.
				ground->ground // surface
			);

			if (motion)
			{
				motion->on_ground = true; // ground->on_ground();
			}

			// Debugging related:
			//motion->apply_gravity = false;
		}

		if (motion)
		{
			// TODO: Look into whether this should be a separate component, etc.
			if (motion->align_to_ground)
			{
				Entity alignment_entity = entity; // null;

				if (auto* alignment_proxy = registry.try_get<AlignmentProxyComponent>(entity))
				{
					if (alignment_proxy->entity != null)
					{
						alignment_entity = alignment_proxy->entity;
					}
				}

				if (alignment_entity != null)
				{
					auto alignment_tform = world.get_transform(alignment_entity);

					auto yaw = alignment_tform.get_yaw();

					alignment_tform.set_basis(surface.alignment());
					alignment_tform.rotateY(yaw, false);
				}
			}
		}
	}

	Entity MotionSystem::detach_motion_proxy(Entity entity, MotionComponent& motion, Entity _opt_new_proxy)
	{
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

			print("Detached.");

			{
				auto tform = world.get_transform(entity);
				print("BEFORE DETACH: {}", tform.get_position());
			}

			// Restore the old/intended parent entity.
			world.set_parent(entity, attachment_proxy.intended_parent);

			{
				auto tform = world.get_transform(entity);
				print("AFTER DETACH: {}", tform.get_position());
			}

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
		auto& registry = world.get_registry();

		auto entity = to_ground.entity();
		
		const auto& ground = registry.get<GroundComponent>(entity);
		auto new_ground_entity = to_ground.ground();

		// In the event this is the same ground entity we're currently attached to, return immediately. (Sanity check)
		if ((new_ground_entity == ground.entity()) && (ground.on_ground()))
		{
			return;
		}

		// Handle dynamic ground attachment:
		if (auto* motion = registry.try_get<MotionComponent>(entity))
		{
			if ((motion->attach_to_dynamic_ground) && to_ground.is_dynamic_ground())
			{
				attach_motion_proxy(entity, *motion, new_ground_entity);
			}
		}
	}

	void MotionSystem::on_ground_to_air(const OnGroundToAir& to_air)
	{
		auto& registry = world.get_registry();
		
		auto entity = to_air.entity();

		auto* motion = registry.try_get<MotionComponent>(entity);

		if (!motion)
		{
			return;
		}

		motion->on_ground = false; // to_air.surface.is_contacted;

		if (to_air.was_dynamic_ground() && (motion->is_attached))
		{
			/*
			auto parent = world.get_parent(entity);
			auto parent_name = world.get_name(parent);
			auto parent_tform = world.get_transform(parent);
			auto parent_position = parent_tform.get_position();

			auto entity_tform = world.get_transform(entity);
			auto entity_position = entity_tform.get_position();

			print("parent_position: {} ({})", parent_position, parent_position.y);
			print("entity_position: {} ({})", entity_position, entity_position.y);

			print("Y Diff: {}", (entity_position.y-parent_position.y));

			print("WOULD DETACH");

			return;
			*/

			detach_motion_proxy(entity, *motion);
		}
	}

	std::optional<CollisionCastResult> MotionSystem::detect_air
	(
		Entity entity,
		Transform& transform,
		GroundComponent& ground_comp,
		const CollisionComponent& collision_comp,
		const GravityComponent& gravity_comp,

		float delta
	)
	{
		auto& registry = get_registry();

		if (!ground_comp.on_ground())
		{
			// Already in the air.
			return std::nullopt;
		}

		/*
		// Reactive version:
		const auto* collision_obj = collision_comp.get_collision_object();

		if (!collision_obj)
		{
			return std::nullopt;
		}

		auto prev_position = math::to_vector(collision_obj->getWorldTransform().getOrigin());
		auto new_position = transform.get_position();
		*/

		// Proactive version:
		const auto gravity = gravity_comp.get_vector(get_gravity(), delta); // transform.align_vector(...);

		const auto prev_position = transform.get_position();
		const auto new_position  = (prev_position + gravity);

		auto ground_check = physics.cast_to(collision_comp, new_position);

		if (ground_check)
		{
			// INSERT GROUND ALIGNMENT LOGIC HERE. (or in `on_surface_contact`...? - not sure yet)
		}
		else
		{
			ground_comp.set_on_ground(false);

			const auto projected_movement = (new_position - prev_position);

			// NOTE: The reverse scenario, `OnAirToGround` occurs in the `on_surface_contact` routine.
			world.event<OnGroundToAir>
			(
				//data.entity,
				ground_comp.ground,

				new_position,
				projected_movement
			);
		}

		return ground_check;
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

	void MotionSystem::accelerate(Entity entity, const math::Vector3D& direction, float influence)
	{
		auto& registry = get_registry();

		auto [velocity_comp, accel_comp] = registry.get<VelocityComponent, AccelerationComponent>(entity);

		bool on_ground = true; // false;

		if (auto* ground_comp = registry.try_get<GroundComponent>(entity))
		{
			on_ground = ground_comp->on_ground();
		}

		const auto speed = velocity_comp.speed();

		const auto acceleration = (on_ground)
			? accel_comp.ground[speed]
			: accel_comp.air[speed]
		;

		velocity_comp.velocity += (direction * (acceleration * influence));

		// Mark `VelocityComponent` as patched.
		registry.patch<VelocityComponent>(entity);
	}

	void MotionSystem::influence_motion_direction(Entity entity, const math::Vector3D& direction, float influence)
	{
		auto& registry = get_registry();

		registry.patch<VelocityComponent>
		(
			entity,

			[&](VelocityComponent& velocity_comp)
			{
				const auto velocity_length = velocity_comp.speed();
				const auto velocity_direction = velocity_comp.direction();

				auto turn_speed = influence;

				if (auto directional_comp = registry.try_get<DirectionalInfluenceComponent>(entity))
				{
					bool on_ground = true; // false;

					if (auto* ground_comp = registry.try_get<GroundComponent>(entity))
					{
						on_ground = ground_comp->on_ground();
					}

					turn_speed *= (on_ground)
						? directional_comp->ground[velocity_length]
						: directional_comp->air[velocity_length]
					;
				}

				const auto influence_updated_direction = math::nlerp(velocity_direction, direction, turn_speed);
				const auto influence_updated_velocity = (influence_updated_direction * velocity_length);

				velocity_comp.velocity = influence_updated_velocity;
			}
		);
	}
}