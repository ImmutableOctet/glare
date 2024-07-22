#pragma once

#include <cstdint>

#include <util/enum_operators.hpp>

namespace engine
{
	enum class LightType : std::uint8_t
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
	
	FLAG_ENUM(std::uint8_t, LightType);
}