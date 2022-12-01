#pragma once

#include "types.hpp"

namespace engine
{
	// Triggers when a state change takes place.
	struct OnStateChange
	{
		Entity entity;

		EntityStateInfo from;
		EntityStateInfo to;

		// Indicates whether the state described by `to` has already been activated.
		// 
		// For most state-change scenarios, this value will be true, meaning the initial changes
		// imposed by switching to the new state have been applied. -- However, a state can utilize
		// a self-imposed delay in activation, which will delay changes for a certain amount of time.
		bool state_activated = true;
	};

	// Triggers after a state has finished activating.
	struct OnStateActivate
	{
		Entity entity;

		EntityStateInfo state;
	};
}