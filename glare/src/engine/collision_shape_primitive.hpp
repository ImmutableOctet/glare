#pragma once

// TODO: Migrate this file into `physics` submodule.

#include <string_view>

namespace engine
{
	enum class CollisionShapePrimitive
	{
		Cube,
		Sphere,
		Capsule,
	};

	// Retrieves a `CollisionShapePrimitive` value based on the input `shape_name`.
	CollisionShapePrimitive collision_shape_primitive(std::string_view shape_name);
}