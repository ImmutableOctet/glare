#pragma once

#include "math_types.hpp"

#include <vector>
#include <variant>

namespace graphics
{
	using VectorArray = std::vector<Vector>;
	using MatrixArray = std::vector<Matrix>;
	using FloatArray  = std::vector<float>;

	using FloatValues = std::variant<float*, FloatArray*>;
}