#pragma once

#include "entity_thread_control_flow_action.hpp"

namespace engine
{
	struct EntityThreadStopAction : EntityThreadControlFlowAction
	{
		bool operator==(const EntityThreadStopAction&) const noexcept = default;
		bool operator!=(const EntityThreadStopAction&) const noexcept = default;
	};
}