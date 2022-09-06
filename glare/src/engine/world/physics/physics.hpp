#pragma once

#include "types.hpp"

#include <util/memory.hpp>

#include <engine/events.hpp>
//#include <engine/world/world_events.hpp>
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
	struct OnTransformChange;

	class PhysicsSystem : public WorldSystem
	{
		public:
			using CollisionWorldRaw = btDiscreteDynamicsWorld;
			//using CollisionWorldRaw = btCollisionWorld;

			static constexpr float MIN_SPEED = 0.0025f;

			PhysicsSystem(World& world, math::Vector gravity={ 0.0f, -1.0f, 0.0f });
			~PhysicsSystem();

			void on_subscribe(World& world) override;
			void on_update(World& world, float delta) override;

			void on_transform_change(const OnTransformChange& tform_change);
			
			void on_create_collider(Registry& registry, Entity entity);
			void on_destroy_collider(Registry& registry, Entity entity); // const CollisionComponent&

			void update_collision_object(CollisionComponent& col, Transform& transform);
			void update_collision_object(btCollisionObject& obj, Transform& transform);

			math::Vector get_gravity() const;
			void set_gravity(const math::Vector& g);
		protected:
			void update_collision_world(float delta);
			void resolve_intersections();
			void update_motion(Entity entity, Transform& transform, PhysicsComponent& ph, float delta);
			void update_collision_object(btCollisionObject& obj, const math::Matrix& m);
		private:
			// Internal routine that handles 'Bullet-to-Engine' synchronization for `ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL`.
			void retrieve_bullet_transforms();

			World& world;

			math::Vector gravity;

			std::unique_ptr<btDefaultCollisionConfiguration> collision_configuration;
			std::unique_ptr<btCollisionDispatcher> collision_dispatcher;
			std::unique_ptr<btDbvtBroadphase> broadphase;
			
			std::unique_ptr<btSequentialImpulseConstraintSolver> solver;

			std::unique_ptr<CollisionWorldRaw> collision_world;
	};
}