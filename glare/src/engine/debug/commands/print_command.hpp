#pragma once

#include <engine/command.hpp>

#include <string>

namespace engine
{
	struct PrintCommand : public Command
	{
		std::string message;
	};
}