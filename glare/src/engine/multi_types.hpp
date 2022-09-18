#pragma once

/*
	General-purpose container type aliases. (e.g. array types)
*/

//#include <types.hpp>
#include "types.hpp"

#include <math/math.hpp>

#include <vector>

namespace engine
{
	using Spline = std::vector<math::Vector>;
	using Entities = std::vector<Entity>;
	using EntityTypes = std::vector<EntityType>;
}