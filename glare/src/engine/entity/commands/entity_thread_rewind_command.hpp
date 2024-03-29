#pragma once

#include <engine/command.hpp>
#include <engine/entity/types.hpp>
#include <engine/entity/actions/entity_thread_rewind_action.hpp>

#include <optional>

namespace engine
{
	struct EntityThreadRewindCommand :
		public Command,
		public EntityThreadRewindAction
	{
		bool operator==(const EntityThreadRewindCommand&) const noexcept = default;
		bool operator!=(const EntityThreadRewindCommand&) const noexcept = default;
	};
}