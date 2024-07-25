#pragma once

#include "types.hpp"

namespace engine
{
	enum class PlayerIndex : PlayerIndexRaw
	{
		None         = 0,
		
		PrimaryLocal = 1,

		Any          = 0,
	};

	inline constexpr PlayerIndex NO_PLAYER            = PlayerIndex::None;
	inline constexpr PlayerIndex PRIMARY_LOCAL_PLAYER = PlayerIndex::PrimaryLocal;
	inline constexpr PlayerIndex ANY_PLAYER           = PlayerIndex::Any;
}