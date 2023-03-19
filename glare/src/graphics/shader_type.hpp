#pragma once

#include <cstdint>

namespace graphics
{
	enum class ShaderType : std::uint8_t
	{
		Vertex,
		Fragment,
		Geometry,
		Tessellation
	};
}