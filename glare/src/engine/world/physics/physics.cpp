#include "physics.hpp"
#include "physics_component.hpp"

#include "collision.hpp"
#include "collision_component.hpp"
#include "collision_events.hpp"
#include "collision_motion_state.hpp"

#include "collision_cast.hpp"

#include "kinematic_resolution_config.hpp"

#include "bullet_util/bullet_util.hpp"

#include <engine/world/world.hpp>
#include <engine/world/world_events.hpp>

#include <engine/transform.hpp>
//#include <engine/relationship.hpp>

#include <math/bullet.hpp>
#include <util/variant.hpp>

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

#include <glm/glm.hpp>

#include <variant>
#include <cmath>
#include <type_traits>

namespace app
{
	struct Graphics;
}

namespace engine
{
	// PhysicsSystem:
	PhysicsSystem::PhysicsSystem(World& world)
	:
		world(world),

		collision_configuration(std::make_unique<btDefaultCollisionConfiguration>()),
		collision_dispatcher(std::make_unique<btCollisionDispatcher>(collision_configuration.get())),
		broadphase(std::make_unique<btDbvtBroadphase>()),

		//solver(std::make_unique<>()),
		solver(std::make_unique<btSequentialImpulseConstraintSolver>()),

		//collision_world(std::make_unique<btSimpleDynamicsWorld>(collision_dispatcher.get(), broadphase.get(), solver.get(), collision_configuration.get()))
		collision_world(std::make_unique<btDiscreteDynamicsWorld>(collision_dispatcher.get(), broadphase.get(), solver.get(), collision_configuration.get()))
		//collision_world(std::make_unique<btCollisionWorld>(collision_dispatcher.get(), broadphase.get(), collision_configuration.get()))
	{
		set_physics_gravity(world.get_gravity());

		//collision_world->setSynchronizeAllMotionStates(true);

		world.subscribe(*this);
	}

	PhysicsSystem::~PhysicsSystem()
	{
		unsubscribe(world);
	}

	void PhysicsSystem::on_subscribe(World& world)
	{
		auto& registry = world.get_registry();

		world.register_event<OnTransformChanged, &PhysicsSystem::on_transform_change>(*this);
		world.register_event<OnGravityChanged, &PhysicsSystem::on_gravity_change>(*this);

		registry.on_construct<CollisionComponent>().connect<&PhysicsSystem::on_create_collider>(*this);
		registry.on_destroy<CollisionComponent>().connect<&PhysicsSystem::on_destroy_collider>(*this);
	}

	// TODO: We need to look at `OnTransformChanged`/`on_transform_change` and how it relates to the collision side of this routine.
	void PhysicsSystem::on_update(World& world, float delta)
	{
		auto& registry = world.get_registry();

		update_collision_world(delta);

		auto view = registry.view<PhysicsComponent>(); // <TransformComponent, ...> (auto& tf_comp)

		// Apply motion (gravity, velocity, deceleration, etc.) and resolve collisions between old and new positions:
		view.each([&](auto entity, auto& ph)
		{
			auto transform = world.get_transform(entity);

			update_motion(entity, transform, ph, delta);
		});
	}

	void PhysicsSystem::on_render(World& world, app::Graphics& graphics)
	{
		collision_world->debugDrawWorld();
	}

	void PhysicsSystem::register_debug_drawer(btIDebugDraw& dbg_draw) // BulletDebugDrawer&
	{
		collision_world->setDebugDrawer(&dbg_draw);
	}

	void PhysicsSystem::unregister_debug_drawer()
	{
		collision_world->setDebugDrawer(nullptr);
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

		resolve_intersections();
	}

	void PhysicsSystem::resolve_intersections()
	{
		return;

		auto& dispatcher = *collision_dispatcher;

		auto manifold_count = dispatcher.getNumManifolds();

		if (manifold_count == 0)
		{
			return;
		}

		//print("manifold_count: {}", manifold_count);

		for (auto i = 0; i < manifold_count; i++)
		{
			auto* contact = dispatcher.getManifoldByIndexInternal(i);

			int numContacts = contact->getNumContacts();

			auto* a = contact->getBody0();
			auto* b = contact->getBody1();

			auto a_ent = get_entity_from_collision_object(*a);
			auto b_ent = get_entity_from_collision_object(*b);

			//print("{} ({}) within bounds of {} ({})", static_cast<entt::id_type>(a_ent), world.get_name(a_ent), static_cast<entt::id_type>(b_ent), world.get_name(b_ent));

			//world.event<OnAABBOverlap>();

			if (numContacts == 0)
			{
				continue;
			}

			//if ((int)a_ent == 34 || (int)b_ent == 34)
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

				//print("Camera is at: {}", math::to_vector(a->getWorldTransform().getOrigin()));
				//print("{} ({}) collision with {} ({})", static_cast<entt::id_type>(a_ent), world.get_name(a_ent), static_cast<entt::id_type>(b_ent), world.get_name(b_ent));

				math::Vector correction = {};

				auto a_tranform = world.get_transform(a_ent);
				//auto a_position = a_tranform.get_position();

				for (int j = 0; j < numContacts; j++)
				{
					btManifoldPoint& pt = contact->getContactPoint(j);

					//auto a_point = math::to_vector(pt.getPositionWorldOnA());
					//auto b_point = math::to_vector(pt.getPositionWorldOnB());
					auto distance = pt.getDistance(); // Equivalent to: glm::length((b_point - a_point));

					if (distance < 0.0f)
					{
						//auto a_point_from_center = (a_point - a_position);
						//auto b_point_from_a_center = (b_point - a_position);
						//auto b_point_from_a_point = (b_point - a_point);

						auto normal = math::to_vector(pt.m_normalWorldOnB); // glm::normalize(...);

						//auto push_back_len = glm::length(b_point_from_a_center);
						//auto push_back_len = glm::length(b_point_from_a_point);
						
						auto length = -distance; // * 1.5f;
						//auto length = (-distance) + collision_world->getDispatchInfo().m_allowedCcdPenetration;

						//length = std::max(length, 0.5f);

						correction += (normal * length);
					}
				}

				// Average the correction-amount by the number of contacts.
				correction /= numContacts;

				a_tranform.move(correction);

				//*/
			}
		}
	}

	void PhysicsSystem::on_gravity_change(const OnGravityChanged& gravity)
	{
		// Forward the world's new gravity to Bullet.
		set_physics_gravity(gravity.new_gravity);
	}

	void PhysicsSystem::on_transform_change(const OnTransformChanged& tform_change)
	{
		auto entity = tform_change.entity;

		auto& registry = world.get_registry();

		auto* col = registry.try_get<CollisionComponent>(entity);

		if (!col)
		{
			return;
		}

		auto* motion_state = col->get_motion_state();

		if (motion_state)
		{
			#if defined(ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL) && (ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL == 1)
				// Handle motion-state submission/update to Bullet:
				// We perform this check before submission in order to reduce the overhead of calling `get_matrix`.
				if (motion_state->can_submit_to_bullet())
				{
					auto transform = world.get_transform(entity);

					motion_state->submit_to_bullet(transform.get_matrix());
				}
			#else
				/*
				// Disabled for now; Bullet will call into `motion_state`'s
				// set `setWorldTransform` method every time, causing another
				// `OnTransformChanged` event-trigger continuously.
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
			// Kinematic logic:
			auto* collision_obj = col->get_collision_object();

			if (!collision_obj)
			{
				return;
			}

			auto transform = world.get_transform(entity);

			if (col->is_active() && col->is_kinematic())
			{
				handle_transform_resolution(entity, *col, *collision_obj, transform);
			}

			//collision_world->convexSweepTest();
			//collision_world->contactTest();

			update_collision_object_transform(*collision_obj, transform.get_matrix());

			// Debugging related:
			//auto t = col->get_collision_object()->getWorldTransform();
			//print("Position is now: {}", math::to_vector(t.getOrigin()));
		}
	}

	void PhysicsSystem::handle_transform_resolution
	(
		Entity entity, CollisionComponent& collision,
		btCollisionObject& collision_obj,
		Transform& transform
	)
	{
		const auto kinematic_resolution_opt = collision.get_kinematic_resolution();

		if (!kinematic_resolution_opt.has_value())
		{
			return;
		}

		const auto& kinematic_resolution = *kinematic_resolution_opt;

		// TODO: Look into reducing number of casts performed via delta length-check.
		//auto position_delta = glm::length(math::to_vector(to_position) - math::to_vector(from_position));

		switch (kinematic_resolution.cast_method)
		{
			case CollisionCastMethod::ConvexCast:
			{
				auto result = convex_cast_to(*this, entity, transform.get_matrix());

				if (result)
				{
					const auto* hit_object = result->native.hit_object;
					const auto  hit_entity = result->hit_entity;

					//auto fraction = callback.m_closestHitFraction;

					auto hit_normal = result->hit_normal;
					auto hit_point_in_world = result->hit_position;

					//float obj_size = 0.0f;
					auto half_obj_size = kinematic_resolution.get_half_size(collision);

					auto corrected_dest_position = (hit_point_in_world + (hit_normal * half_obj_size));

					//auto attempted_move_length = glm::length(math::to_vector(to_position) - math::to_vector(from_position));
					//auto actual_move_length = glm::length(corrected_dest_position - math::to_vector(from_position));

					transform.set_position(corrected_dest_position);

					// Notify listeners that this `entity` contacted `hit_entity`'s surface.
					world.event<OnCollision>(entity, hit_entity, hit_point_in_world, hit_normal, 0.0f, ContactType::Surface);
				}

				break;
			}
			//case CollisionCastMethod::ConvexKinematicCast:
				// INSERT USE OF `btKinematicClosestNotMeConvexResultCallback` here.
				//break;

			case CollisionCastMethod::RayCast:
				// TODO: Implement ray cast approach. (offset by half of AABB/shape width from contact point)
				//assert(false);
				break;
			
			//case CollisionCastMethod::None:
			default:
				break;
		}
	}

	

	void PhysicsSystem::update_motion(Entity entity, Transform& transform, PhysicsComponent& ph, float delta)
	{
		math::Vector movement = {};

		if (ph.apply_gravity())
		{
			movement += (get_gravity() * delta);
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

		transform.move(movement);
	}

	void PhysicsSystem::on_create_collider(Registry& registry, Entity entity)
	{
		//auto& world = new_col.world;

		auto transform = world.get_transform(entity);
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

	void PhysicsSystem::on_destroy_collider(Registry& registry, Entity entity)
	{
		auto* component = registry.try_get<CollisionComponent>(entity);

		if (!component)
		{
			return;
		}

		auto* collision_obj = component->get_collision_object();

		if (!collision_obj)
		{
			return;
		}

		/*
			Bullet handles checks for derived collision-object types and handles them
			appropriately, so we only need to call `removeCollisionObject` here.
			
			e.g. if the object is actually of type `btRigidBody`, Bullet
			will call `removeRigidBody` automatically.
		*/
		collision_world->removeCollisionObject(collision_obj);
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

	// Internal shorthand for `world.get_gravity()`.
	math::Vector PhysicsSystem::get_gravity() const
	{
		return world.get_gravity();
	}

	// Internal shorthand for `world.down()`.
	math::Vector PhysicsSystem::down() const
	{
		return world.down();
	}

	void PhysicsSystem::set_physics_gravity(const math::Vector& gravity)
	{
		collision_world->setGravity(math::to_bullet_vector(gravity));
	}

	math::Vector PhysicsSystem::get_physics_gravity() const
	{
		return math::to_vector(collision_world->getGravity());
	}

	// Internal routine, used for `ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL`.
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