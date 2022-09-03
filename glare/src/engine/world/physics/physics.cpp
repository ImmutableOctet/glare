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

		//collision_world->setSynchronizeAllMotionStates(true);

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
		world.register_event<OnComponentAdd<CollisionComponent>, &PhysicsSystem::on_new_collider>(*this);
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

		print("manifold_count: {}", manifold_count);

		for (auto i = 0; i < manifold_count; i++)
		{
			auto* contact = dispatcher.getManifoldByIndexInternal(i);

			int numContacts = contact->getNumContacts();

			auto* a = contact->getBody0();
			auto* b = contact->getBody1();

			auto a_ent = get_entity_from_collision_object(*a);
			auto b_ent = get_entity_from_collision_object(*b);

			print("{} ({}) within bounds of {} ({})", static_cast<entt::id_type>(a_ent), world.get_name(a_ent), static_cast<entt::id_type>(b_ent), world.get_name(b_ent));

			if (numContacts == 0)
			{
				continue;
			}

			if ((int)a_ent == 34 || (int)b_ent == 34)
			{
				//auto t = world.get_transform(a_ent);
				//update_collision_object(const_cast<btCollisionObject&>(*a), t);

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

				///*

				print("Camera is at: {}", math::to_vector(a->getWorldTransform().getOrigin()));

				print("{} ({}) collision with {} ({})", static_cast<entt::id_type>(a_ent), world.get_name(a_ent), static_cast<entt::id_type>(b_ent), world.get_name(b_ent));

				math::Vector v = {};

				auto a_tranform = world.get_transform(a_ent);
				auto a_position = a_tranform.get_position();

				for (int j = 0; j < numContacts; j++)
				{
					btManifoldPoint& pt = contact->getContactPoint(j);

					//if (pt.getDistance() < 0.f)
					{
						auto ptA = math::to_vector(pt.getPositionWorldOnA());
						auto ptB = math::to_vector(pt.getPositionWorldOnB());

						//auto contact_dist = -glm::normalize(ptA - ptB) * 1.005f * math::to_vector(pt.m_normalWorldOnB); // glm::normalize(math::to_vector(pt.m_normalWorldOnB));


						//const btVector3& normalOnB = pt.m_normalWorldOnB;

						auto contact_dist = (a_position - math::to_vector(ptA));

						v += contact_dist;
					}
				}

				v /= numContacts;

				a_tranform.move(v);

				//*/
			}
		}
	}

	void PhysicsSystem::on_transform_change(const OnTransformChange& tform_change)
	{
		auto entity = tform_change.entity;

		auto& registry = world.get_registry();

		auto* col = registry.try_get<CollisionComponent>(entity);

		if (!col)
		{
			return;
		}

		auto transform = world.get_transform(entity);

		// Debugging related (camera):
		if ((int)tform_change.entity == 34)
		{
			//print("Entity changed: {} @ {}", tform_change.entity, transform.get_position());
			
			/*
			bool is_kinematic = col->get_rigid_body()->isKinematicObject();
			bool is_static = col->get_rigid_body()->isStaticObject();
			bool is_static_or_kinematic = col->get_rigid_body()->isStaticOrKinematicObject();
			*/

			//auto t = col->get_collision_object()->getWorldTransform();
			//print("Collision object position: {}", math::to_vector(t.getOrigin()));
		}
		
		//auto* collision_obj = col->collision.get();

		//auto from = collision_obj->getWorldTransform();
		//auto to = math::to_bullet_matrix(transform.get_matrix());

		//collision_world->convexSweepTest();
		//collision_world->contactTest();

		auto* motion_state = col->get_motion_state();

		if (motion_state)
		{
			#if defined(ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL) && (ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL == 1)
				// Handle motion-state submission/update to Bullet:
				// We perform this check before submission in order to reduce the overhead of calling `get_matrix`.
				if (motion_state->can_submit_to_bullet())
				{
					motion_state->submit_to_bullet(transform.get_matrix());
				}
			#else
				/*
				// Disabled for now; Bullet will call into `motion_state`'s
				// set `setWorldTransform` method every time, causing another
				// `OnTransformChange` event-trigger continuously.
				if (motion_state)
				{
					auto* rigid_body = col->get_rigid_body();

					if (rigid_body)
					{
						collision_world->synchronizeSingleMotionState(rigid_body);
					}
				}
				*/
			#endif
		}
		else
		{
			update_collision_object(*col, transform);

			// Debugging related:
			//auto t = col->get_collision_object()->getWorldTransform();
			//print("Position is now: {}", math::to_vector(t.getOrigin()));
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
		update_collision_object_transform(obj, m);
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

	math::Vector PhysicsSystem::get_gravity() const
	{
		//return math::to_vector(collision_world->getGravity());

		return gravity;
	}

	void PhysicsSystem::set_gravity(const math::Vector& g)
	{
		gravity = g;

		// TODO: Determine if we need/want to use Bullet's built-in gravity functionality.
		//collision_world->setGravity(math::to_bullet_vector(g));
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