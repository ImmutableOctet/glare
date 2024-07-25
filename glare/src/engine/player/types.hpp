#pragma once

#include <cstdint>

namespace engine
{
	// NOTE: This must be equivalent to `app::input::PlayerInputID`.
	using PlayerIndexRaw = std::uint16_t;

	using PlayerCount = PlayerIndexRaw;

	using PlayerFlagsRaw = std::uint8_t;

	enum class PlayerIndex : PlayerIndexRaw;
	enum class PlayerFlags : PlayerFlagsRaw;

	inline constexpr PlayerCount MAX_PLAYERS = 32;
}