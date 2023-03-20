#pragma once

#include "entity_thread_control_flow_action.hpp"

namespace engine
{
	struct EntityThreadPauseAction : EntityThreadControlFlowAction
	{
		bool operator==(const EntityThreadPauseAction&) const noexcept = default;
		bool operator!=(const EntityThreadPauseAction&) const noexcept = default;
	};
}