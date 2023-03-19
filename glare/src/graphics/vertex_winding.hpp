#pragma once

#include <cstdint>

namespace graphics
{
	enum class VertexWinding : std::uint8_t
	{
		Clockwise,
		CounterClockwise,
	};
}