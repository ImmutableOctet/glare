#pragma once

#include <engine/command.hpp>
#include <engine/entity/types.hpp>
#include <engine/entity/actions/entity_thread_stop_action.hpp>

#include <optional>

namespace engine
{
	struct EntityThreadStopCommand :
		public Command,
		public EntityThreadStopAction
	{};
}