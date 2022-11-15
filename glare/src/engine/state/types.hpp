#pragma once

#include <engine/types.hpp>

#include <optional>

namespace engine
{
	struct EntityStateInfo
	{
		EntityStateIndex index;
		std::optional<EntityStateHash> id;
	};
}