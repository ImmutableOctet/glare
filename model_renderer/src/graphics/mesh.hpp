#pragma once

#include <vector>
#include <cstdint>

#include "vertex.hpp"

namespace graphics
{
	class Mesh
	{
		using index_t = std::int32_t;

		template <typename vertex_t>
		struct Data
		{
			std::vector<vertex_t> vertices;
			std::vector<index_t> indices;
		};
	};
}