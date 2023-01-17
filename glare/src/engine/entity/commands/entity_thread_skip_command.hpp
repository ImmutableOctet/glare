#pragma once

#include <engine/command.hpp>
#include <engine/entity/types.hpp>
#include <engine/entity/actions/entity_thread_skip_action.hpp>

#include <optional>

namespace engine
{
	struct EntityThreadSkipCommand :
		public Command,
		public EntityThreadSkipAction
	{};
}