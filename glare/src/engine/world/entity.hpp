#pragma once

#include <engine/types.hpp>
//#include <engine/relationship.hpp>
#include <engine/transform.hpp>

namespace engine
{
	Entity create_entity(World& world, Entity parent=null, EntityType type={});
	void destory_entity(World& world, Entity entity, bool destroy_orphans=true);

	Entity create_pivot(World& world, Entity parent=null, EntityType type=EntityType::Pivot);
	Entity create_pivot(World& world, const math::Vector& position, Entity parent=null, EntityType type=EntityType::Pivot);
	Entity create_pivot(World& world, const math::Vector& position, const math::Vector& rotation, const math::Vector& scale={1.0f, 1.0f, 1.0f}, Entity parent=null, EntityType type=EntityType::Pivot);

	inline Entity create_pivot(World& world, const math::TransformVectors& tform, Entity parent=null, EntityType type=EntityType::Pivot)
	{
		auto [position, rotation, scale] = tform;

		return create_pivot(world, position, rotation, scale, parent, type);
	}
}