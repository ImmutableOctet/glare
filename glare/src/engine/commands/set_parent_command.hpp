#pragma once

#include <engine/command.hpp>

namespace engine
{
	struct SetParentCommand : Command
	{
		Entity parent;
	};
}