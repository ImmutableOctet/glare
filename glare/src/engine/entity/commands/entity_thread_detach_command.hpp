#pragma once

#include <engine/command.hpp>
#include <engine/entity/types.hpp>
#include <engine/entity/actions/entity_thread_detach_action.hpp>

#include <optional>

namespace engine
{
	struct EntityThreadDetachCommand :
		public Command,
		public EntityThreadDetachAction
	{
		bool operator==(const EntityThreadDetachCommand&) const noexcept = default;
		bool operator!=(const EntityThreadDetachCommand&) const noexcept = default;
	};
}