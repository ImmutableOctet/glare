#pragma once

#include <engine/types.hpp>
//#include <engine/relationship.hpp>
//#include <engine/transform.hpp>

#include <filesystem>
#include <util/json.hpp>

// TODO: Change this API into something generic enough to work for multiple Services, not just `World`.

namespace engine
{
	class World;
	class ResourceManager;

	Entity create_entity(World& world, Entity parent=null, EntityType type={});

	Entity create_pivot(World& world, Entity parent=null, EntityType type=EntityType::Pivot);
	Entity create_pivot(World& world, const math::Vector& position, Entity parent=null, EntityType type=EntityType::Pivot);
	Entity create_pivot(World& world, const math::Vector& position, const math::Vector& rotation, const math::Vector& scale={1.0f, 1.0f, 1.0f}, Entity parent=null, EntityType type=EntityType::Pivot);

	inline Entity create_pivot(World& world, const math::TransformVectors& tform, Entity parent=null, EntityType type=EntityType::Pivot)
	{
		auto [position, rotation, scale] = tform;

		return create_pivot(world, position, rotation, scale, parent, type);
	}
}