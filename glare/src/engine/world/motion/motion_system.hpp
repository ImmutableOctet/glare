#pragma once

#include <engine/types.hpp>
#include <engine/world/world_system.hpp>
#include <engine/world/physics/collision_cast_result.hpp>

#include <optional>

namespace engine
{
	class World;
	class PhysicsSystem;

	struct Transform;
	struct MotionComponent;
	struct CollisionComponent;

	struct OnSurfaceContact;
	struct OnAirToGround;
	struct OnGroundToAir;

	// Kinematic/logic-driven motion system, this system is complimentary to `PhysicsSystem`,
	// which handles both general-purpose collision functionality and simulated dynamic bodies.
	// `MotionSystem` is responsible for things like player, enemy and object motion which is driven by game logic.
	class MotionSystem : public WorldSystem
	{
		public:
			struct EntityData
			{
				Entity entity;
				
				Transform& transform;
				MotionComponent& motion;
				CollisionComponent& collision;

				float delta;
			};

			// TODO: Also look at orientation when determining down vector for surface response values.
			
			// Ground threshold.
			static constexpr float GROUND = 0.65f; // (Dot-product value)
			
			// Wall threshold.
			//static constexpr float WALL = ...; // (Dot-product value)

			static constexpr float MIN_SPEED = 0.0025f;

			MotionSystem(World& world, PhysicsSystem& physics);

			void on_subscribe(World& world) override;
			void on_update(World& world, float delta) override;

			// TODO: Handle surface contact behaviors (alignment, etc.).
			void on_surface_contact(const OnSurfaceContact& surface);

			void on_air_to_ground(const OnAirToGround& to_ground);
			void on_ground_to_air(const OnGroundToAir& to_air);

			// Detaches `entity` from its current motion proxy.
			// If no proxy is currently established, this will safely return `null`.
			Entity detach_motion_proxy(Entity entity, MotionComponent& motion, Entity _opt_new_proxy=null);

			// Attaches an `entity` to a motion `proxy`, allowing its
			// transform to be manipulated via scene-graph inheritance.
			// If `entity` is already attached to another motion proxy, the previous proxy will be detached automatically.
			// If attachment fails, this returns `null`. (Usually caused by attaching to the same object)
			Entity attach_motion_proxy(Entity entity, MotionComponent& motion, Entity proxy);

			std::optional<CollisionCastResult> detect_air
			(
				const EntityData& data,
				const math::Vector& gravity_movement,
				const math::Vector& projected_movement
			);

			void update_motion(const EntityData& data);

			// Internal shorthand for `world.get_gravity()`.
			math::Vector get_gravity() const;

			// Internal shorthand for `world.down()`.
			math::Vector down() const;
		protected:
			PhysicsSystem& physics;
	};
}