#pragma once

// TODO: Remove in favor of forward declaration header.
#include <engine/types.hpp>

#include <util/enum_operators.hpp>

namespace engine
{
	enum class LightType : LightTypeRaw
	{
		// Allows for multiple light sub-components
		// to be bound to the same entity.
		Any,

		// Specific types:
		Point,
		Directional,
		Spot,

		Spotlight = Spot,
	};
	
	FLAG_ENUM(LightTypeRaw, LightType);
}