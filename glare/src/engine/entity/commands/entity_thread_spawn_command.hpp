#pragma once

#include <engine/command.hpp>
#include <engine/entity/types.hpp>
#include <engine/entity/actions/entity_thread_spawn_action.hpp>

#include <optional>

namespace engine
{
	struct EntityThreadSpawnCommand :
		public Command,
		public EntityThreadSpawnAction
	{
		std::optional<EntityStateIndex> state_index = std::nullopt;
	};
}