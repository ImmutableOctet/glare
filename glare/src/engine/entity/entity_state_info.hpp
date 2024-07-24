#pragma once

#include "types.hpp"

#include <optional>

namespace engine
{
	struct EntityStateInfo
	{
		EntityStateIndex index;
		std::optional<EntityStateHash> id; // EntityStateHash
	};
}