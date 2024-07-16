#pragma once

#include <engine/entity/entity_thread_target.hpp>

namespace engine
{
	struct EntityThreadSpawnAction
	{
		EntityThreadTarget threads;

		EntityThreadID parent_thread_name = {};

		bool restart_existing = false;

		bool operator==(const EntityThreadSpawnAction&) const noexcept = default;
		bool operator!=(const EntityThreadSpawnAction&) const noexcept = default;
	};
}