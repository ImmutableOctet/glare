#pragma once

#include <engine/types.hpp>
#include <engine/command.hpp>

namespace engine
{
	// Performs a 'manual' state activation.
	// (Usually performed in conjunction with a timed-event)
	// 
	// NOTE: This does not handle updating the target entity's `StateComponent`,
	// as activation is intended to happen after an official 'state-change' has taken place.
	//
	// For general use-cases, see: `StateChangeCommand`
	struct StateActivationCommand : public Command
	{
		StringHash state_name;
	};
}