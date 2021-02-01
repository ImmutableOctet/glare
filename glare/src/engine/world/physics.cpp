#include "physics.hpp"
#include "world.hpp"

#include <engine/types.hpp>
#include <math/bullet.hpp>

//#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/BulletCollision/btBulletCollisionCommon.h>

#include <glm/glm.hpp>
#include <cmath>

namespace engine
{
	// PhysicsSystem:
	PhysicsSystem::PhysicsSystem(math::Vector gravity)
	:
		gravity(gravity), // <-- This is set later anyway, but just in case something depends on the raw value during construction, we set it here as well.

		collision_configuration(std::make_unique<btDefaultCollisionConfiguration>()),
		collision_dispatcher(std::make_unique<btCollisionDispatcher>(collision_configuration.get())),
		broadphase(std::make_unique<btDbvtBroadphase>()),

		//solver(std::make_unique<btSequentialImpulseConstraintSolver>()),

		//collision_world(std::make_unique<btDiscreteDynamicsWorld>(collision_dispatcher.get(), broadphase.get(), solver.get(), collision_configuration.get()))
		collision_world(std::make_unique<btCollisionWorld>(collision_dispatcher.get(), broadphase.get(), collision_configuration.get()))
	{
		set_gravity(gravity);
	}

	PhysicsSystem::~PhysicsSystem()
	{
		// Nothing so far.
	}

	void PhysicsSystem::update(World& world, float dt)
	{
		auto& registry = world.get_registry();

		auto view = registry.view<PhysicsComponent>(); // <TransformComponent, ...> (auto& tf_comp)

		auto& gravity = this->gravity;

		// Apply motion (gravity, velocity, deceleration, etc.):
		view.each([&](auto entity, auto& ph)
		{
			auto transform = world.get_transform(entity);

			math::Vector movement = {};

			if (ph.apply_gravity())
			{
				movement += (gravity * ph.gravity() * dt);
			}

			if (ph.apply_velocity())
			{
				movement += (ph.motion.velocity * dt);
			}

			auto decel = std::abs(ph.motion.deceleration);

			if (decel > 0.0f)
			{
				if (glm::length(ph.motion.velocity) > MIN_SPEED)
				{
					ph.motion.velocity -= (ph.motion.velocity * decel * dt); // math::Vector
				}
				else
				{
					ph.motion.velocity = {};
				}
			}

			ph.motion.prev_position = transform.get_position();

			transform.move(movement);
		});


	}

	void PhysicsSystem::set_gravity(const math::Vector& g)
	{
		gravity = g;

		//collision_world->setGravity(math::to_bullet_vector(g));
	}

	void PhysicsSystem::on_new_collider(World& world, OnComponentAdd<CollisionComponent>& new_col)
	{
		//auto& world = new_col.world;
		auto& entity = new_col.entity;

		auto transform = world.get_transform(entity);

		auto& registry = world.get_registry();

		auto& component = registry.get<CollisionComponent>(entity);
		auto* collision_obj = component.collision.get(); // new_col.component

		update_collision_object(*collision_obj, transform.get_matrix());
	}

	void PhysicsSystem::update_collision_object(btCollisionObject& obj, const math::Matrix& m)
	{
		obj.setWorldTransform(math::to_bullet_matrix(m));
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