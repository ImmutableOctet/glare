#include "physics.hpp"
#include "physics_component.hpp"

#include <engine/world/world.hpp>

#include <engine/collision.hpp>
#include <engine/transform.hpp>
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
		// Debugging related (camera):
		if ((int)tform_change.entity == 34)
		{
			//print("Entity changed: {}", tform_change.entity);
		}

		auto entity = tform_change.entity;

		auto& registry = world.get_registry();

		auto* col = registry.try_get<CollisionComponent>(entity);

		if (!col)
		{
			return;
		}

		if (!col->collision)
		{
			return;
		}

		auto transform = world.get_transform(entity);
		
		//auto* collision_obj = col->collision.get();

		//auto from = collision_obj->getWorldTransform();
		//auto to = math::to_bullet_matrix(transform.get_matrix());

		//collision_world->convexSweepTest();
		//collision_world->contactTest();

		update_collision_object(transform, *col);
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

		update_collision_world(delta);

		auto view = registry.view<PhysicsComponent>(); // <TransformComponent, ...> (auto& tf_comp)
		
		auto& gravity = this->gravity;

		// Apply motion (gravity, velocity, deceleration, etc.) and resolve collisions between old and new positions:
		view.each([&](auto entity, auto& ph)
		{
			auto transform = world.get_transform(entity);

			update_motion(entity, transform, ph, delta);
		});
	}

	void PhysicsSystem::update_collision_world(float delta)
	{
		collision_world->updateAabbs();
		collision_world->computeOverlappingPairs();

		collision_world->performDiscreteCollisionDetection();
		//collision_world->stepSimulation(...);

		auto& dispatcher = *collision_dispatcher;

		auto manifold_count = dispatcher.getNumManifolds();

		if (manifold_count == 0)
		{
			return;
		}

		for (auto i = 0; i < manifold_count; i++)
		{
			auto* contact = dispatcher.getManifoldByIndexInternal(i);

			auto* a = contact->getBody0();
			auto* b = contact->getBody1();

			auto a_ent = get_entity_from_collision_object(*a);
			auto b_ent = get_entity_from_collision_object(*b);

			if ((int)a_ent == 34 || (int)b_ent == 34)
			{
				print("{} ({}) collision with {} ({})", static_cast<entt::id_type>(a_ent), world.get_name(a_ent), static_cast<entt::id_type>(b_ent), world.get_name(b_ent));
			}
		}
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

	void PhysicsSystem::update_motion(Entity entity, Transform& transform, PhysicsComponent& ph, float delta)
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
}