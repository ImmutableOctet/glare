#pragma once

#include <engine/command.hpp>

#include <string>

namespace engine
{
	// NOTE: It is up to the game to implement a handler for this command/event.
	// For 'default' handling, use the `DebugListener` type.
	struct PrintCommand : public Command
	{
		std::string message;
	};
}