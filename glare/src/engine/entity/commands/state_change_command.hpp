#pragma once

#include <engine/types.hpp>
#include <engine/command.hpp>

namespace engine
{
	struct StateChangeCommand : public Command
	{
		StringHash state_name;

		bool operator==(const StateChangeCommand&) const noexcept = default;
		bool operator!=(const StateChangeCommand&) const noexcept = default;
	};
}