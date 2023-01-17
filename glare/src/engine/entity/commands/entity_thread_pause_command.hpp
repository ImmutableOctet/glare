#pragma once

#include <engine/command.hpp>
#include <engine/entity/types.hpp>
#include <engine/entity/entity_thread_pause_action.hpp>

#include <optional>

namespace engine
{
	struct EntityThreadPauseCommand :
		public Command,
		public EntityThreadPauseAction
	{};
}