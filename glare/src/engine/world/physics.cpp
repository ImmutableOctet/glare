#include "physics.hpp"
#include "world.hpp"

#include <engine/types.hpp>
#include <engine/transform.hpp>
#include <engine/collision.hpp>
//#include <engine/relationship.hpp>

#include <math/bullet.hpp>

//#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletCollisionCommon.h>
//#include <bullet/BulletCollision/btBulletCollisionCommon.h>

#include <glm/glm.hpp>
#include <cmath>

namespace engine
{
	// PhysicsSystem:
	PhysicsSystem::PhysicsSystem(World& world, math::Vector gravity)
	:
		world(world),
		gravity(gravity), // <-- This is set later anyway, but just in case something depends on the raw value during construction, we set it here as well.

		collision_configuration(std::make_unique<btDefaultCollisionConfiguration>()),
		collision_dispatcher(std::make_unique<btCollisionDispatcher>(collision_configuration.get())),
		broadphase(std::make_unique<btDbvtBroadphase>()),

		//solver(std::make_unique<btSequentialImpulseConstraintSolver>()),

		//collision_world(std::make_unique<btDiscreteDynamicsWorld>(collision_dispatcher.get(), broadphase.get(), solver.get(), collision_configuration.get()))
		collision_world(std::make_unique<btCollisionWorld>(collision_dispatcher.get(), broadphase.get(), collision_configuration.get()))
	{
		set_gravity(gravity);

		world.subscribe(*this);
	}

	PhysicsSystem::~PhysicsSystem()
	{
		unsubscribe(world);
	}

	void PhysicsSystem::on_subscribe(World& world)
	{
		world.register_event<OnTransformChange, &PhysicsSystem::on_transform_change>(*this);
		world.register_event<OnEntityDestroyed, &PhysicsSystem::on_entity_destroyed>(*this);
		world.register_event<OnComponentAdd<engine::CollisionComponent>, &PhysicsSystem::on_new_collider>(*this);
	}

	void PhysicsSystem::on_transform_change(const OnTransformChange& tform_change)
	{
		print("Entity changed: {}", tform_change.entity);
	}

	void PhysicsSystem::on_entity_destroyed(const OnEntityDestroyed& destruct)
	{
		auto& registry = world.get_registry();

		auto entity = destruct.entity;

		// Handle collision:
		auto* col = registry.try_get<CollisionComponent>(entity);

		if (col)
		{
			on_destroy_collider(entity, *col);
		}
	}

	// TODO: We need to look at `OnTransformChange`/`on_transform_change` and how it relates to the collision side of this routine.
	void PhysicsSystem::on_update(World& world, float delta)
	{
		auto& registry = world.get_registry();

		collision_world->updateAabbs();
		collision_world->computeOverlappingPairs();

		auto view = registry.view<PhysicsComponent>(); // <TransformComponent, ...> (auto& tf_comp)
		
		auto& gravity = this->gravity;

		// Apply motion (gravity, velocity, deceleration, etc.) and resolve collisions between old and new positions:
		view.each([&](auto entity, auto& ph)
		{
			auto transform = world.get_transform(entity);

			auto* col = registry.try_get<CollisionComponent>(entity);

			// TODO: See notes below about using bullet's matrices to handle position deltas between physics steps.
			auto from_matrix = transform.get_matrix();

			update_motion(world, entity, transform, ph, delta);

			if (transform.collision_invalid())
			{
				if (col)
				{
					// TODO: We could probably use the bullet side's matrix for `CollisionComponent` here, rather than forcing all movement to be performed by `update_motion`.
					auto from = math::to_bullet_matrix(from_matrix);

					auto to   = math::to_bullet_matrix(transform.get_matrix());

					// Check-for/resolve collisions during movement.
					//collision_world->convexSweepTest(col->);

					//col->collision;

					update_collision_object(transform, *col);
				}
			}
		});
	}

	void PhysicsSystem::set_gravity(const math::Vector& g)
	{
		gravity = g;

		// TODO: Determine if we need/want to use Bullet's built-in gravity functionality.
		//collision_world->setGravity(math::to_bullet_vector(g));
	}

	void PhysicsSystem::on_new_collider(const OnComponentAdd<CollisionComponent>& new_col)
	{
		//auto& world = new_col.world;
		auto entity = new_col.entity;

		auto transform = world.get_transform(entity);

		auto& registry = world.get_registry();

		auto& component = registry.get<CollisionComponent>(entity);
		auto* collision_obj = component.collision.get(); // new_col.component

		if (!collision_obj) // component.collision
		{
			return;
		}

		auto c_group = static_cast<int>(component.get_group());
		auto c_mask = static_cast<int>(component.get_full_mask());

		collision_world->addCollisionObject(collision_obj, c_group, c_mask);

		//update_collision_object(*collision_obj, transform.get_matrix());
		update_collision_object(transform, component);
	}

	void PhysicsSystem::on_destroy_collider(Entity entity, CollisionComponent& col)
	{
		if (col.collision)
		{
			auto* collision_obj = col.collision.get();

			collision_world->removeCollisionObject(collision_obj);
		}
	}

	void PhysicsSystem::update_collision_object(Transform& transform, CollisionComponent& col) // World& world, Entity entity
	{
		if (col.collision)
		{
			auto* collision_obj = col.collision.get();

			update_collision_object(*collision_obj, transform.get_matrix());
		}
	}

	void PhysicsSystem::update_collision_object(btCollisionObject& obj, const math::Matrix& m)
	{
		obj.setWorldTransform(math::to_bullet_matrix(m));
	}

	void PhysicsSystem::update_motion(World& world, Entity entity, Transform& transform, PhysicsComponent& ph, float delta)
	{
		math::Vector movement = {};

		if (ph.apply_gravity())
		{
			movement += (gravity * ph.gravity() * delta);
		}

		if (ph.apply_velocity())
		{
			movement += (ph.motion.velocity * delta);
		}

		auto decel = std::abs(ph.motion.deceleration);

		if (decel > 0.0f)
		{
			if (glm::length(ph.motion.velocity) > MIN_SPEED)
			{
				ph.motion.velocity -= (ph.motion.velocity * decel * delta); // math::Vector
			}
			else
			{
				ph.motion.velocity = {};
			}
		}

		ph.motion.prev_position = transform.get_position();

		transform.move(movement);
	}

	// PhysicsComponent:
	PhysicsComponent::PhysicsComponent(PhysicsComponent::Flags motion_flags, MotionState motion_state)
		: flags(motion_flags), motion(motion_state)
	{}

	Entity attach_physics(World& world, Entity entity, PhysicsComponent::Flags flags, PhysicsComponent::MotionState motion_state)
	{
		auto& registry = world.get_registry();

		registry.emplace_or_replace<PhysicsComponent>(entity, flags, motion_state);

		return entity;
	}
}