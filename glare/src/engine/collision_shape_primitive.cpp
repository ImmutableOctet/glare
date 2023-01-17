// TODO: Move to a different location or remove.

#include "collision_shape_primitive.hpp"

#include <entt/core/hashed_string.hpp>
#include <util/string.hpp>

#include "meta/meta.hpp"

namespace engine
{
	CollisionShapePrimitive collision_shape_primitive(std::string_view shape_name)
	{
		using namespace entt::literals;

		using enum CollisionShapePrimitive;

		auto shape_name_processed = util::lowercase(shape_name);

		switch (hash(shape_name_processed.c_str()))
		{
			case "capsule"_hs:
				return Capsule;
			case "cube"_hs:
				return Cube;
			case "sphere"_hs:
				return Sphere;
		}

		return Capsule;
	}
}