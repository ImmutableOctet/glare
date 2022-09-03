#include "collision_motion_state.hpp"
#include "collision_config.hpp"

#include <engine/world/world.hpp>
#include <engine/transform.hpp>

#include <math/bullet.hpp>

#include <bullet/btBulletDynamicsCommon.h>

namespace engine
{
	#if defined(ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL) && (ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL == 1)
		_CollisionMotionState::_CollisionMotionState(World& world, Entity entity)
			: direction(DIRECTION_WRITE_TO_BULLET)
		{
			auto transform = world.get_transform(entity);

			this->matrix = math::to_bullet_matrix(transform.get_matrix());
		}

		void _CollisionMotionState::getWorldTransform(btTransform& worldTrans) const
		{
			// Always report the cached value when requested by Bullet.
			worldTrans = this->matrix;
		}

		void _CollisionMotionState::setWorldTransform(const btTransform& worldTrans)
		{
			// Always take new values from Bullet and flag down the game engine accordingly:
			this->matrix = worldTrans;

			// This will allow `retrieve_from_bullet` to complete the next time it's called.
			direction = DIRECTION_WRITE_TO_ENGINE;
		}

		bool _CollisionMotionState::submit_to_bullet(const math::Matrix& matrix)
		{
			// If Bullet has already submitted a transform to the engine, don't override the internal cache.
			if (direction == DIRECTION_WRITE_TO_ENGINE)
			{
				return false;
			}

			this->matrix = math::to_bullet_matrix(matrix);

			// Update the internal state to account for the new transform.
			direction = DIRECTION_WRITE_TO_BULLET;

			// Notify the user that we were successful.
			return true;
		}

		bool _CollisionMotionState::can_submit_to_bullet() const
		{
			return (direction == DIRECTION_WRITE_TO_BULLET);
		}

		std::optional<math::Matrix> _CollisionMotionState::retrieve_from_bullet()
		{
			// If we're still in the 'DIRECTION_WRITE_TO_BULLET' state (from a previous call),
			// then Bullet hasn't reported anything new, and we can return immediately:
			if (direction == DIRECTION_WRITE_TO_BULLET)
			{
				return std::nullopt;
			}

			// Set the state back to `DIRECTION_WRITE_TO_BULLET` and await an update.
			direction = DIRECTION_WRITE_TO_BULLET;

			// Return the matrix reported by Bullet.
			return math::to_matrix(matrix);
		}
	#else
		_CollisionMotionState::_CollisionMotionState(World& world, Entity entity)
			: world(world), entity(entity)
		{
			assert(world.get_registry().try_get<TransformComponent>(entity));
		}

		void _CollisionMotionState::getWorldTransform(btTransform& worldTrans) const
		{
			auto tform = world.get_transform(entity);

			worldTrans = math::to_bullet_matrix(tform.get_matrix());
		}

		void _CollisionMotionState::setWorldTransform(const btTransform& worldTrans)
		{
			auto tform = world.get_transform(entity);
			
			tform.set_matrix(math::to_matrix(worldTrans));
		}
	#endif

	std::unique_ptr<CollisionMotionState> make_collision_motion_state(World& world, Entity entity, const CollisionConfig& config)
	{
		if (config.solid_mask == CollisionGroup::None)
		{
			return {};
		}

		// TODO: May change this later.
		/*
		if (!(config.group & CollisionGroup::DynamicGeometry)) // && !(config.group & CollisionGroup::Projectile)
		{
			return {};
		}
		*/

		// If using regular implementations of `CollisionMotionState`:
		return std::make_unique<CollisionMotionState>(world, entity);

		/*
		// If using `btDefaultMotionState` in place of `CollisionMotionState`:
		auto transform = world.get_transform(entity);
		auto bullet_matrix = math::to_bullet_matrix(transform.get_matrix());

		return std::make_unique<CollisionMotionState>(bullet_matrix);
		*/
	}
}