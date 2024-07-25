#pragma once

#include "types.hpp"

#include <util/small_vector.hpp>

namespace engine
{
	template <typename T, std::size_t max_players=MAX_PLAYERS>
	struct PlayerRelative
	{
		util::small_vector<T, max_players> data;
	};
}