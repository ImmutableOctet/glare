#pragma once

#include "entity_thread_control_flow_action.hpp"

namespace engine
{
	struct EntityThreadSkipAction : EntityThreadControlFlowAction
	{
		EntityInstructionCount instructions_skipped = 0;

		bool operator==(const EntityThreadSkipAction&) const noexcept = default;
		bool operator!=(const EntityThreadSkipAction&) const noexcept = default;
	};
}