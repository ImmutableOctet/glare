#pragma once

#include <engine/command.hpp>
#include <engine/entity/types.hpp>
#include <engine/entity/entity_thread_rewind_action.hpp>

#include <optional>

namespace engine
{
	struct EntityThreadRewindCommand :
		public Command,
		public EntityThreadRewindAction
	{};
}