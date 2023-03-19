#pragma once

#include <util/enum_operators.hpp>

#include <cstdint>

namespace graphics
{
	enum class ContextFlags : std::uint32_t
	{
		None             = (1 << 0),
		DepthTest        = (1 << 1),
		FaceCulling      = (1 << 2),
		VSync            = (1 << 3),
		Wireframe        = (1 << 4),

		Default          = (DepthTest|FaceCulling|VSync),
	};
	
	FLAG_ENUM(std::uint32_t, ContextFlags);
}