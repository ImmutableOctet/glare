#pragma once

#include "entity_thread_control_flow_action.hpp"

namespace engine
{
	struct EntityThreadDetachAction : EntityThreadControlFlowAction
	{
		bool operator==(const EntityThreadDetachAction&) const noexcept = default;
		bool operator!=(const EntityThreadDetachAction&) const noexcept = default;
	};
}