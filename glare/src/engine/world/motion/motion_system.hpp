#pragma once

#include <engine/types.hpp>
#include <engine/world/world_system.hpp>

namespace engine
{
	class World;
	class PhysicsSystem;

	struct Transform;
	struct MotionComponent;
	struct CollisionComponent;

	struct OnSurfaceContact;

	// Kinematic/logic-driven motion system, this system is complimentary to `PhysicsSystem`,
	// which handles both general-purpose collision functionality and simulated dynamic bodies.
	// `MotionSystem` is responsible for things like player, enemy and object motion which is driven by game logic.
	class MotionSystem : public WorldSystem
	{
		public:
			// TODO: Also look at orientation when determining down vector for surface response values.
			// Ground threshold.
			//static constexpr float GROUND = ...; // (Dot product value)
			
			// Wall threshold.
			//static constexpr float WALL = ...; // (Dot product value)

			static constexpr float MIN_SPEED = 0.0025f;

			MotionSystem(World& world, PhysicsSystem& physics);

			void on_subscribe(World& world) override;
			void on_update(World& world, float delta) override;

			// TODO: Handle surface contact behaviors (alignment, etc.).
			//void on_surface_contact(const OnSurfaceContact& surface);

			void update_motion
			(
				Entity entity,
				Transform& transform,
				MotionComponent& motion,
				CollisionComponent& collision,
				float delta
			);

			// Internal shorthand for `world.get_gravity()`.
			math::Vector get_gravity() const;

			// Internal shorthand for `world.down()`.
			math::Vector down() const;
		protected:
			PhysicsSystem& physics;
	};
}