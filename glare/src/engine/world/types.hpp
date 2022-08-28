#pragma once

#include <engine/types.hpp>

namespace engine
{
	using PlayerIndex = std::uint16_t;
	constexpr PlayerIndex PRIMARY_LOCAL_PLAYER = 1;

	enum class Character : std::uint8_t
	{
		Glare = 0,

		Default = Glare
	};
}