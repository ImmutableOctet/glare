#pragma once

// TODO: Look into performance differences between standard implementation and alternative.
// TODO: Determine if alternative implementation would work better in multi-threaded contexts.
//#define ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL 1

#include <engine/types.hpp>

//#include <bullet/btBulletCollisionCommon.h>
#include <bullet/LinearMath/btMotionState.h>

#include <memory>

#if defined(ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL) && (ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL == 1)
	#include <optional>
#endif

class btTransform;

namespace engine
{
	class World;

	struct CollisionConfig;

	/*
		This synchronizes changes in the world-space transform of
		collision-objects between Bullet and the game engine.
	
		NOTE:
		When passing a pointer to this class to Bullet, be sure to give persistent storage to this object;
		e.g. allocate via `std::make_unique`. -- This is needed due to entt's ability to relocate/reallocate components.

		TODO: Determine if there's any way to optimize this via existing transform change-detection mechanisms.
	*/
	class CollisionMotionState : public btMotionState
	{
		protected:
			#if defined(ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL) && (ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL == 1)
				// We have no need to reference a `World` or `Entity` explicitly, as `CollisionMotionState` objects
				// are normally held by `CollisionComponent` instances, making them already associated.
				
				// The last known internal matrix state of the represented entity.
				btTransform matrix;

				// The direction we're currently operating with. (see below)
				bool direction;
			#else
				// The world this motion-state exists within.
				World& world;

				// The entity this motion-state represents.
				Entity entity;
			#endif
		public:
			CollisionMotionState(World& world, Entity entity);

			// Bullet interface methods:

			/*
				If using the standard implementation:
				This will construct a temporary `Transform` object in order to resolve the world-space matrix for `entity`.

				If using the alternate implementation:
				This will simply report the last known world-space matrix reported by a `PhysicsSystem`.
			*/
			virtual void getWorldTransform(btTransform& worldTrans) const override;

			/*
				If using the standard implementation:
				This will construct a temporary `Transform` object in order
				to assign the world-space matrix of `entity`.

				If using the alternate implementation:
				This will assign an internal world-space matrix variable and await the
				next update from a `PhysicsSystem` in order to apply the result. (see `retrieve_from_bullet`)
			*/
			virtual void setWorldTransform(const btTransform& worldTrans) override;

			// Alternate implementation details:
			#if defined(ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL) && (ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL == 1)
				static constexpr bool DIRECTION_WRITE_TO_BULLET = false;
				static constexpr bool DIRECTION_WRITE_TO_ENGINE = true;

				bool submit_to_bullet(const math::Matrix& matrix);
				bool can_submit_to_bullet() const;

				// If available, this retrieves the newest matrix reported by Bullet.
				// Once this method is called, the internal state of this
				// object will be set in the 'write-to-Bullet' direction.
				// Note: This method is normally called automatically by a `PhysicsSystem`.
				std::optional<math::Matrix> retrieve_from_bullet();
			#endif
	};

	std::unique_ptr<CollisionMotionState> make_collision_motion_state(World& world, Entity entity, const CollisionConfig& config);
}