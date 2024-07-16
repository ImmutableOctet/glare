#pragma once

#include <engine/command.hpp>

#include <engine/entity/types.hpp>
#include <engine/entity/entity_thread_flags.hpp>

#include <engine/script/script_fiber.hpp>
#include <engine/script/script_handle.hpp>

#include <optional>

namespace engine
{
	struct EntityThreadFiberSpawnCommand :
		public Command
	{
		ScriptFiber fiber;
		
		std::optional<EntityStateIndex> state_index;

		EntityThreadID parent_thread_name = {};
		EntityThreadID thread_name = {};

		EntityThreadFlags thread_flags = {};

		ScriptHandle script_handle = {};

		bool operator==(const EntityThreadFiberSpawnCommand&) const noexcept = default;
		bool operator!=(const EntityThreadFiberSpawnCommand&) const noexcept = default;

		inline bool has_fiber() const
		{
			return static_cast<bool>(fiber);
		}

		inline explicit operator bool() const
		{
			return has_fiber();
		}
	};
}