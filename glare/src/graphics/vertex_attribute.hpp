#pragma once

#include "element_type.hpp"

namespace graphics
{
	struct VertexAttribute
	{
		ElementType type;

		int num_elements; // unsigned int
		int offset = 0;
	};
}