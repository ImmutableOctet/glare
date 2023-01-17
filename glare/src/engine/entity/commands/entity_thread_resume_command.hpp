#pragma once

#include <engine/command.hpp>
#include <engine/entity/types.hpp>
#include <engine/entity/entity_thread_resume_action.hpp>

#include <optional>

namespace engine
{
	struct EntityThreadResumeCommand :
		public Command,
		public EntityThreadResumeAction
	{};
}