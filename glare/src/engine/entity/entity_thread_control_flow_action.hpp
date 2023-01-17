#pragma once

#include "types.hpp"

//#include "entity_thread_range.hpp"
#include "entity_thread_target.hpp"

#include <optional>

namespace engine
{
	struct EntityThreadControlFlowAction
	{
		EntityThreadTarget threads;

		bool check_linked = true;
	};
}