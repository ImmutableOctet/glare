#pragma once

#include "entity_thread_control_flow_action.hpp"

namespace engine
{
	struct EntityThreadResumeAction : EntityThreadControlFlowAction
	{
		bool operator==(const EntityThreadResumeAction&) const noexcept = default;
		bool operator!=(const EntityThreadResumeAction&) const noexcept = default;
	};
}