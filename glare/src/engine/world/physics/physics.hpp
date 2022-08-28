#pragma once

#include "types.hpp"

#include <util/memory.hpp>

#include <engine/events/events.hpp>
//#include <engine/collision.hpp>
//#include <engine/transform.hpp>

#include <engine/world/world_system.hpp>

// Forward declarations:

// Bullet:
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
struct btDbvtBroadphase;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btCollisionWorld;
class btCollisionObject;
class btCollisionShape;

namespace engine
{
	struct Transform;
	struct PhysicsComponent;
	struct CollisionComponent;

	class PhysicsSystem : public WorldSystem
	{
		public:
			static constexpr float MIN_SPEED = 0.0025f;

			PhysicsSystem(World& world, math::Vector gravity={ 0.0f, -1.0f, 0.0f });
			~PhysicsSystem();

			void on_subscribe(World& world) override;

			void on_transform_change(const OnTransformChange& tform_change);
			void on_entity_destroyed(const OnEntityDestroyed& destruct);

			void on_update(World& world, float delta) override;

			inline math::Vector get_gravity() const { return gravity; }

			void set_gravity(const math::Vector& g);

			void on_new_collider(const OnComponentAdd<CollisionComponent>& new_col);
			void on_destroy_collider(Entity entity, CollisionComponent& col); // const CollisionComponent&
			void update_collision_object(Transform& transform, CollisionComponent& col);
		protected:
			void update_collision_world(float delta);
			void update_motion(Entity entity, Transform& transform, PhysicsComponent& ph, float delta);
			void update_collision_object(btCollisionObject& obj, const math::Matrix& m);
		private:
			World& world;

			math::Vector gravity;

			std::unique_ptr<btDefaultCollisionConfiguration> collision_configuration;
			std::unique_ptr<btCollisionDispatcher> collision_dispatcher;
			std::unique_ptr<btDbvtBroadphase> broadphase;
			
			//std::unique_ptr<btSequentialImpulseConstraintSolver> solver;

			//std::unique_ptr<btDiscreteDynamicsWorld> collision_world;
			std::unique_ptr<btCollisionWorld> collision_world;
	};
}