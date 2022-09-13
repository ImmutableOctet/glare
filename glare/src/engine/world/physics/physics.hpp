#pragma once

#include "types.hpp"
#include "collision_cast_result.hpp"

#include <util/memory.hpp>

#include <engine/events.hpp>
//#include <engine/world/world_events.hpp>
#include <engine/world/world_system.hpp>

#include <optional>

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

class btIDebugDraw;

namespace graphics
{
	class BulletDebugDrawer;
}

namespace engine
{
	struct Transform;
	struct PhysicsComponent;
	struct CollisionComponent;
	struct OnTransformChanged;
	struct OnGravityChanged;

	class PhysicsSystem : public WorldSystem
	{
		public:
			static constexpr float MIN_SPEED = 0.0025f;

			PhysicsSystem(World& world);
			~PhysicsSystem();

			void on_subscribe(World& world) override;
			void on_update(World& world, float delta) override;
			void on_render(World& world, app::Graphics& graphics) override;

			void register_debug_drawer(btIDebugDraw& dbg_draw); // BulletDebugDrawer&
			void unregister_debug_drawer();

			void on_gravity_change(const OnGravityChanged& gravity);

			void on_transform_change(const OnTransformChanged& tform_change);
			
			void on_create_collider(Registry& registry, Entity entity);
			void on_destroy_collider(Registry& registry, Entity entity); // const CollisionComponent&

			void update_collision_object(CollisionComponent& col, Transform& transform);
			void update_collision_object(btCollisionObject& obj, Transform& transform);

			inline World& get_world() const { return world; }

			inline auto* get_collision_world() { return collision_world.get(); }
			inline auto* get_broadphase() { return broadphase.get(); }
			inline auto* get_collision_dispatcher() { return collision_dispatcher.get(); }
		protected:
			void update_collision_world(float delta);

			void handle_transform_resolution
			(
				Entity entity, CollisionComponent& collision,
				btCollisionObject& collision_obj,
				Transform& transform
			);

			void resolve_intersections();
			
			void update_motion(Entity entity, Transform& transform, PhysicsComponent& ph, float delta);
			void update_collision_object(btCollisionObject& obj, const math::Matrix& m);

			// Internal shorthand for `world.get_gravity()`.
			math::Vector get_gravity() const;

			// Internal shorthand for `world.down()`.
			math::Vector down() const;
		private:
			// Manually sets the physics system's gravity. (this does not impact `world`)
			void set_physics_gravity(const math::Vector& gravity);

			// Retrieves the gravity value managed by the internally held `collision_world`.
			math::Vector get_physics_gravity() const;

			// Internal routine that handles 'Bullet-to-Engine' synchronization for `ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL`.
			void retrieve_bullet_transforms();

			World& world;

			std::unique_ptr<btDefaultCollisionConfiguration> collision_configuration;
			std::unique_ptr<btCollisionDispatcher> collision_dispatcher;
			std::unique_ptr<btDbvtBroadphase> broadphase;
			
			std::unique_ptr<btSequentialImpulseConstraintSolver> solver;

			std::unique_ptr<btDiscreteDynamicsWorld> collision_world; // btCollisionWorld
	};
}