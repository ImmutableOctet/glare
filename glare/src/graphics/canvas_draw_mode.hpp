#pragma once

#include <util/enum_operators.hpp>

#include <cstdint>

namespace graphics
{
	enum class CanvasDrawMode : std::uint32_t
	{
		None            = (1 << 0),

		Opaque          = (1 << 1),
		Transparent     = (1 << 2),

		// If enabled, the active shader will not be checked before drawing.
		IgnoreShaders   = (1 << 3),
		IgnoreTextures  = (1 << 4),
		IgnoreMaterials = (1 << 5),

		// When rendering a scene normally, this disables receiving shadows.
		IgnoreShadows   = (1 << 6),

		_Shadow         = (1 << 7),
		Shadow          = _Shadow | (Opaque|IgnoreShaders|IgnoreTextures|IgnoreMaterials), // Transparent
		//Shadow        = (1 << 7),

		All = (Opaque | Transparent),
	};
	
	FLAG_ENUM(std::uint32_t, CanvasDrawMode);
}