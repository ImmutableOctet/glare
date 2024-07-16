#pragma once

#include <engine/entity/entity_thread_target.hpp>

namespace engine
{
	struct EntityThreadUnlinkAction
	{
		EntityThreadTarget threads;

		bool operator==(const EntityThreadUnlinkAction&) const noexcept = default;
		bool operator!=(const EntityThreadUnlinkAction&) const noexcept = default;
	};
}