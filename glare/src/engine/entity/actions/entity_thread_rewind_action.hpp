#pragma once

#include "entity_thread_control_flow_action.hpp"

namespace engine
{
	struct EntityThreadRewindAction : EntityThreadControlFlowAction
	{
		EntityInstructionCount instructions_rewound = {};

		bool operator==(const EntityThreadRewindAction&) const noexcept = default;
		bool operator!=(const EntityThreadRewindAction&) const noexcept = default;
	};
}