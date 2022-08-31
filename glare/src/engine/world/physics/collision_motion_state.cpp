#include "collision_motion_state.hpp"
#include "collision_config.hpp"

#include <engine/world/world.hpp>
#include <engine/transform.hpp>

#include <math/bullet.hpp>

namespace engine
{
	CollisionMotionState::CollisionMotionState(World& world, Entity entity)
		: world(world), entity(entity)
	{
		assert(world.get_registry().try_get<TransformComponent>(entity));
	}

	void CollisionMotionState::getWorldTransform(btTransform& worldTrans) const
	{
		auto tform = world.get_transform(entity);

		worldTrans = math::to_bullet_matrix(tform.get_matrix());
	}

	void CollisionMotionState::setWorldTransform(const btTransform& worldTrans)
	{
		auto tform = world.get_transform(entity);
		
		tform.set_matrix(math::to_matrix(worldTrans));
	}

	std::unique_ptr<CollisionMotionState> make_collision_motion_state(World& world, Entity entity, const CollisionConfig& config)
	{
		if (config.solid_mask == CollisionGroup::None)
		{
			return {};
		}

		// Exact group values:
		switch (config.group)
		{
			case CollisionGroup::StaticGeometry:
			case CollisionGroup::Zone:
			//case CollisionGroup::Particle:
				return {};
		}

		return std::make_unique<CollisionMotionState>(world, entity);
	}
}