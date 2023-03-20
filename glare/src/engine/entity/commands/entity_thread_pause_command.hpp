#pragma once

#include <engine/command.hpp>
#include <engine/entity/types.hpp>
#include <engine/entity/actions/entity_thread_pause_action.hpp>

#include <optional>

namespace engine
{
	struct EntityThreadPauseCommand :
		public Command,
		public EntityThreadPauseAction
	{
		bool operator==(const EntityThreadPauseCommand&) const noexcept = default;
		bool operator!=(const EntityThreadPauseCommand&) const noexcept = default;
	};
}