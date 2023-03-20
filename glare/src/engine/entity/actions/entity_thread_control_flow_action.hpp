#pragma once

#include <engine/entity/entity_thread_target.hpp>

#include <optional>

namespace engine
{
	struct EntityThreadControlFlowAction
	{
		EntityThreadTarget threads;

		bool check_linked = true;

		bool operator==(const EntityThreadControlFlowAction&) const noexcept = default;
		bool operator!=(const EntityThreadControlFlowAction&) const noexcept = default;
	};
}