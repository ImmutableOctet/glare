#include "physics.hpp"
#include "physics_component.hpp"
#include "collision.hpp"
#include "collision_motion_state.hpp"

#include <engine/world/world.hpp>

#include <engine/transform.hpp>
//#include <engine/relationship.hpp>

#include <math/bullet.hpp>

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

#include <glm/glm.hpp>

#include <variant>
#include <cmath>
#include <type_traits>

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

		//solver(std::make_unique<>()),
		solver(std::make_unique<btSequentialImpulseConstraintSolver>()),

		//collision_world(std::make_unique<btSimpleDynamicsWorld>(collision_dispatcher.get(), broadphase.get(), solver.get(), collision_configuration.get()))
		collision_world(std::make_unique<btDiscreteDynamicsWorld>(collision_dispatcher.get(), broadphase.get(), solver.get(), collision_configuration.get()))
		//collision_world(std::make_unique<btCollisionWorld>(collision_dispatcher.get(), broadphase.get(), collision_configuration.get()))
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

		auto transform = world.get_transform(entity);
		
		//auto* collision_obj = col->collision.get();

		//auto from = collision_obj->getWorldTransform();
		//auto to = math::to_bullet_matrix(transform.get_matrix());

		//collision_world->convexSweepTest();
		//collision_world->contactTest();
		
		auto* motion_state = col->get_motion_state();

		if (motion_state)
		{
			// Handle motion-state submission/update to Bullet:
			#if defined(ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL) && (ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL == 1)
				// We perform this check before submission in order to reduce the overhead of calling `get_matrix`.
				if (motion_state->can_submit_to_bullet())
				{
					motion_state->submit_to_bullet(transform.get_matrix());
				}
			#endif
		}
		else
		{
			// This should only really apply to things like event/interaction triggers, non-kinematic geometry, etc.
			// i.e. entities utilizing collision-detection without the need for a solver.
			// For more details, see the conditions outlined in `make_collision_motion_state`.
			update_collision_object(*col, transform);
		}
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
		auto& registry = world.get_registry();

		/*
		collision_world->updateAabbs();
		collision_world->computeOverlappingPairs();
		collision_world->performDiscreteCollisionDetection();
		*/

		collision_world->stepSimulation(delta);

		#if defined(ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL) && (ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL == 1)
			retrieve_bullet_transforms();
		#endif

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
				print("Camera is at: {}", math::to_vector(a->getWorldTransform().getOrigin()));

				print("{} ({}) collision with {} ({})", static_cast<entt::id_type>(a_ent), world.get_name(a_ent), static_cast<entt::id_type>(b_ent), world.get_name(b_ent));

				//auto a_tform = a->getWorldTransform();
				//auto t = world.get_transform(a_ent);
				//t.set_matrix(math::to_matrix(a_tform));

				/*
				auto a_wrapper = btCollisionObjectWrapper(nullptr, a->getCollisionShape(), a, a->getWorldTransform(), 0, 0);
				auto b_wrapper = btCollisionObjectWrapper(nullptr, b->getCollisionShape(), b, b->getWorldTransform(), 0, 0);

				btCollisionAlgorithm* algorithm = collision_world->getDispatcher()->findAlgorithm(&a_wrapper, &b_wrapper, contact, ebtDispatcherQueryType::BT_CLOSEST_POINT_ALGORITHMS);

				if (algorithm)
				{
					auto result = btManifoldResult(&a_wrapper, &b_wrapper);
					algorithm->processCollision(&a_wrapper, &b_wrapper, collision_world->getDispatchInfo(), &result);
				}
				*/

				/*
				int numContacts = contact->getNumContacts();
				for (int j = 0; j < numContacts; j++)
				{
					btManifoldPoint& pt = contact->getContactPoint(j);

					if (pt.getDistance() < 0.f)
					{
						const btVector3& ptA = pt.getPositionWorldOnA();
						const btVector3& ptB = pt.getPositionWorldOnB();
						const btVector3& normalOnB = pt.m_normalWorldOnB;
					}
				}
				*/
			}
		}
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

		bool object_found = component.on_collision_object
		(
			[this, &component, &transform](const auto& collision_obj)
			{
				auto c_group = static_cast<int>(component.get_group());
				auto c_mask = static_cast<int>(component.get_full_mask());

				using obj_type = std::decay_t<decltype(collision_obj)>;

				if constexpr (std::is_same_v<obj_type, CollisionComponent::RigidBody>)
				{
					this->collision_world->addRigidBody(collision_obj.get(), c_group, c_mask);
				}
				else
				{
					this->collision_world->addCollisionObject(collision_obj.get(), c_group, c_mask);
				}

				update_collision_object(*collision_obj, transform);
			}
		);
	}

	void PhysicsSystem::on_destroy_collider(Entity entity, CollisionComponent& col)
	{
		auto* collision_obj = col.get_collision_object();

		if (collision_obj)
		{
			/*
				Bullet handles checks for derived collision-object types and handles them
				appropriately, so we only need to call `removeCollisionObject` here.
			
				e.g. if the object is actually of type `btRigidBody`, Bullet
				will call `removeRigidBody` automatically.
			*/
			collision_world->removeCollisionObject(collision_obj);
		}
	}

	void PhysicsSystem::update_collision_object(CollisionComponent& col, Transform& transform) // World& world, Entity entity
	{
		// TODO: Determine if any addition work needs to be done in this step for other collision-object types. (e.g. `btRigidBody`)

		auto* collision_obj = col.get_collision_object();

		if (collision_obj)
		{
			update_collision_object(*collision_obj, transform);
		}
	}

	void PhysicsSystem::update_collision_object(btCollisionObject& obj, Transform& transform)
	{
		update_collision_object(obj, transform.get_matrix());
	}

	void PhysicsSystem::update_collision_object(btCollisionObject& obj, const math::Matrix& m)
	{
		obj.setWorldTransform(math::to_bullet_matrix(m));
	}

	void PhysicsSystem::retrieve_bullet_transforms()
	{
		#if defined(ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL) && (ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL == 1)
			auto& registry = world.get_registry();

			// Retrieve newly reported transform states from Bullet:
			auto view = registry.view<CollisionComponent>();

			view.each([this](auto entity, CollisionComponent& collision)
			{
				auto* motion_state = collision.get_motion_state();

				if (!motion_state)
				{
					return;
				}

				auto new_matrix = motion_state->retrieve_from_bullet();

				if (new_matrix.has_value())
				{
					auto transform = world.get_transform(entity);

					transform.set_matrix(*new_matrix);
				}
			});
		#endif
	}
}