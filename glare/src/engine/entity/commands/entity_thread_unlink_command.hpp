#pragma once

#include <engine/command.hpp>
#include <engine/entity/types.hpp>
#include <engine/entity/entity_thread_unlink_action.hpp>

#include <optional>

namespace engine
{
	struct EntityThreadUnlinkCommand :
		public Command,
		public EntityThreadUnlinkAction
	{};
}