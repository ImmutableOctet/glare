#pragma once

/*
	General-purpose container type aliases. (e.g. array types)
*/

//#include <types.hpp>
#include "types.hpp"

#include <math/types.hpp>

#include <vector>
#include <variant>

namespace engine
{
	using Spline = std::vector<math::Vector>;
	using Entities = std::vector<Entity>;
	using EntityTypes = std::vector<EntityType>;

	// Alias for variant type of either a single entity or a vector of entities.
	using EntityOrEntities = std::variant<Entity, Entities>;
}