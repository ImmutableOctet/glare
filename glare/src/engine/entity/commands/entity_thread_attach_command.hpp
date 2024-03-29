#pragma once

#include <engine/command.hpp>
#include <engine/entity/types.hpp>
#include <engine/entity/actions/entity_thread_attach_action.hpp>

#include <optional>

namespace engine
{
	struct EntityThreadAttachCommand :
		public Command,
		public EntityThreadAttachAction
	{
		bool operator==(const EntityThreadAttachCommand&) const noexcept = default;
		bool operator!=(const EntityThreadAttachCommand&) const noexcept = default;
	};
}