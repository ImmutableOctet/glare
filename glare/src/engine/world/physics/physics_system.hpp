#pragma once

#include "types.hpp"
#include "collision_cast_result.hpp"
#include "collision_cast.hpp"

#include <util/memory.hpp>

#include <engine/events.hpp>
//#include <engine/world/world_events.hpp>
#include <engine/world/world_system.hpp>

#include <optional>

// Bullet includes required in header due to usage with `entt::any`/`std::any`.
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

// Forward declarations:

// Bullet:
//class btDiscreteDynamicsWorld;
//class btSequentialImpulseConstraintSolver;
class btCollisionWorld;
class btDefaultCollisionConfiguration;
struct btDbvtBroadphase;
class btCollisionDispatcher;
class btCollisionObject;
class btCollisionShape;

class btIDebugDraw;

namespace graphics
{
	class BulletDebugDrawer;
}

namespace engine
{
	// Components:
	struct Transform;
	struct MotionComponent;
	struct CollisionComponent;

	// Events:
	struct OnTransformChanged;
	struct OnGravityChanged;
	struct OnIntersection;
	struct OnSurfaceContact;

	// Physics and collisions system; responsible for collision detection and kinematic resolution.
	class PhysicsSystem : public WorldSystem
	{
		public:
			PhysicsSystem(World& world);
			//~PhysicsSystem();

			PhysicsSystem(const PhysicsSystem&) = delete;

			// TODO: Look into removing this move constructor.
			// This may not be needed, and may have unintended side effects due to subscription.
			PhysicsSystem(PhysicsSystem&&) noexcept = default;

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

			// Performs a collision cast (convex, ray, etc.) from `obj`'s current position to `destination`.
			// This is a utility function for handling dispatch to different casting algorithms.
			// If you know exactly which type of cast you need to perform, use the free-function casting API instead.
			template <typename SelfType>
			std::optional<CollisionCastResult> cast_to
			(
				const SelfType& obj,
				const math::Vector& destination,

				std::optional<CollisionGroup> filter_group=std::nullopt,
				std::optional<CollisionGroup> filter_mask=std::nullopt
			)
			{
				return cast_to_obj_impl(obj, destination, filter_group, filter_mask);
			}

			// Explicit overload for `CollisionComponent`, shared for multiple visit paths.
			// See main `RayCastSelf` overload for details.
			std::optional<CollisionCastResult> cast_to
			(
				const CollisionComponent& collision,
				const math::Vector& destination,

				std::optional<CollisionGroup> filter_group=std::nullopt,
				std::optional<CollisionGroup> filter_mask=std::nullopt,

				bool check_kinematic_resolution=false
			);

			inline auto* get_collision_world() { return collision_world.get(); }
			inline auto* get_broadphase() { return broadphase.get(); }
			inline auto* get_collision_dispatcher() { return collision_dispatcher.get(); }

			// TODO: Change this to a dedicated field.
			inline constexpr auto get_max_ray_distance() const { return 2000.0f; }
		protected:
			std::optional<CollisionCastResult> cast_to_obj_impl
			(
				const impl::RayCastSelf& obj,
				const math::Vector& destination,

				std::optional<CollisionGroup> filter_group=std::nullopt,
				std::optional<CollisionGroup> filter_mask=std::nullopt
			);

			void update_collision_world(float delta);

			void handle_transform_resolution
			(
				Entity entity, CollisionComponent& collision,
				btCollisionObject& collision_obj,
				Transform& transform
			);

			void handle_intersections(bool check_resolution_flags=true);
			
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

			// Event aliasing:

			// Forward declared here, defined in main source file. (Internal)
			template <typename CollisionEventType>
			void forward_collision_event(const CollisionEventType& event_obj);
			
			// Produces an equivalent `OnCollision` event for this `OnSurfaceContact` event.
			void on_surface_contact(const OnSurfaceContact& surface);

			// Produces an equivalent `OnCollision` event for this `OnIntersection` event.
			void on_intersection(const OnIntersection& intersection);

			// Internal routine that handles 'Bullet-to-Engine' synchronization for `ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL`.
			void retrieve_bullet_transforms();

			std::unique_ptr<btDefaultCollisionConfiguration> collision_configuration;
			std::unique_ptr<btCollisionDispatcher> collision_dispatcher;
			std::unique_ptr<btDbvtBroadphase> broadphase;
			std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
			std::unique_ptr<btDiscreteDynamicsWorld> collision_world; // btCollisionWorld
	};
}