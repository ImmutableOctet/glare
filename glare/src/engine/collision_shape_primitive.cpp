#include "collision_shape_primitive.hpp"

#include <entt/core/hashed_string.hpp>
#include <util/string.hpp>

namespace engine
{
	CollisionShapePrimitive collision_shape_primitive(std::string_view shape_name)
	{
		using namespace entt;

		using enum CollisionShapePrimitive;

		auto shape_name_processed = util::lowercase(shape_name);

		switch (hashed_string(shape_name_processed.c_str()))
		{
			case hashed_string("capsule"):
				return Capsule;
			case hashed_string("cube"):
				return Cube;
			case hashed_string("sphere"):
				return Sphere;
		}

		return Capsule;
	}
}