#pragma once

#include "entity_thread_control_flow_action.hpp"

namespace engine
{
	struct EntityThreadAttachAction : EntityThreadControlFlowAction
	{
		std::optional<EntityStateID> state_id = std::nullopt;

		bool operator==(const EntityThreadAttachAction&) const noexcept = default;
		bool operator!=(const EntityThreadAttachAction&) const noexcept = default;
	};
}