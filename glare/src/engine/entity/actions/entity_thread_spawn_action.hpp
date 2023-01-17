#pragma once

#include <engine/entity/entity_thread_target.hpp>

namespace engine
{
	struct EntityThreadSpawnAction
	{
		EntityThreadTarget threads;

		bool restart_existing = false;
	};
}