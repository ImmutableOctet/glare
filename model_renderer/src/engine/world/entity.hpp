#pragma once

#include <engine/types.hpp>
//#include <engine/relationship.hpp>
#include <engine/transform.hpp>

namespace engine
{
	Entity create_entity(World& world, Entity parent=null);
	void destory_entity(World& world, Entity entity);

	Entity create_pivot(World& world, Entity parent=null);
}