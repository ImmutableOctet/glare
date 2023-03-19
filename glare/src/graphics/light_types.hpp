#pragma once

#include "array_types.hpp"
#include "math_types.hpp"

#include <variant>

namespace graphics
{
	using LightPositions = std::variant<Vector*, VectorArray*>; // const
	using LightMatrices  = std::variant<Matrix*, MatrixArray*>;
}