#pragma once

#include "element_type.hpp"

#include <cstdint>

namespace graphics
{
	// TODO: Look into whether we should optimize this.
	struct VertexAttribute
	{
		ElementType type;

		std::int32_t num_elements = 0;
		std::int32_t offset = 0;
	};
}