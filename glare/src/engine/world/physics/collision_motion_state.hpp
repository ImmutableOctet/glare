#pragma once

#include <engine/types.hpp>

//#include <bullet/btBulletCollisionCommon.h>
#include <bullet/LinearMath/btMotionState.h>

#include <memory>

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
			World& world;
			Entity entity;
		public:
			CollisionMotionState(World& world, Entity entity);

			virtual void getWorldTransform(btTransform& worldTrans) const override;
			virtual void setWorldTransform(const btTransform& worldTrans) override;
	};

	std::unique_ptr<CollisionMotionState> make_collision_motion_state(World& world, Entity entity, const CollisionConfig& config);
}