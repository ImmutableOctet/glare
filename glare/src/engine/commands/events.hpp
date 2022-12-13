#pragma once

#include <engine/command.hpp>
#include <engine/meta/types.hpp>

#include <optional>

namespace engine
{
	// This is triggered any time a command is executed. (i.e. triggered)
	struct OnCommandExecution
	{
		Command command;

		std::optional<MetaTypeID> command_id = std::nullopt;
	};
}