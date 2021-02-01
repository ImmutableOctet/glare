#pragma once

#include <engine/types.hpp>
#include <math/math.hpp>
#include <util/memory.hpp>

#include <engine/events/events.hpp>
#include <engine/collision.hpp>

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
	class World;

	enum class MotionFlags : std::uint8_t
	{
		None = 0,

		ApplyGravity = (1 << 0),
		ApplyVelocity = (1 << 2),

		StandardMovement = (ApplyGravity|ApplyVelocity),
	};

	FLAG_ENUM(std::uint8_t, MotionFlags);

	struct PhysicsComponent
	{
		using Flags = MotionFlags;

		struct MotionState
		{
			math::Vector prev_position = {};
			math::Vector velocity = {};

			float gravity_mag = 0.01f; // 1.0f;
			float deceleration = 0.0f;
		};

		PhysicsComponent(Flags motion_flags=(Flags::StandardMovement), MotionState motion_state=MotionState());

		Flags flags;
		MotionState motion;

		inline float gravity() const { return motion.gravity_mag; }

		inline bool apply_gravity() const { return (flags & Flags::ApplyGravity); }
		inline bool apply_velocity() const { return (flags & Flags::ApplyVelocity); }
	};

	class PhysicsSystem
	{
		public:
			static constexpr float MIN_SPEED = 0.0025f;

			PhysicsSystem(math::Vector gravity={ 0.0f, -1.0f, 0.0f });
			~PhysicsSystem();

			void update(World& world, float dt);

			inline math::Vector get_gravity() const { return gravity; }

			void set_gravity(const math::Vector& g);

			void on_new_collider(World& world, OnComponentAdd<CollisionComponent>& new_col);
			void update_collision_object(btCollisionObject& obj, const math::Matrix& m);
		private:
			std::unique_ptr<btDefaultCollisionConfiguration> collision_configuration;
			std::unique_ptr<btCollisionDispatcher> collision_dispatcher;
			std::unique_ptr<btDbvtBroadphase> broadphase;
			
			//std::unique_ptr<btSequentialImpulseConstraintSolver> solver;

			//std::unique_ptr<btDiscreteDynamicsWorld> collision_world;
			std::unique_ptr<btCollisionWorld> collision_world;

			math::Vector gravity;
	};

	Entity attach_physics(World& world, Entity entity, PhysicsComponent::Flags flags, PhysicsComponent::MotionState motion_state = PhysicsComponent::MotionState());
}