#pragma once

#include <engine/types.hpp>

namespace engine
{
	enum class CollisionShapePrimitive : std::uint8_t
	{
		Cube,
		Sphere,
		Capsule,
	};
}