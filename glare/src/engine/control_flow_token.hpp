#pragma once

#include <cstdint>

namespace engine
{
	enum class ControlFlowToken : std::uint8_t
	{
		// Tokens:
		NextUpdate,
		UntilWake,
		Restart,
		//Event,

		Complete,

		// Aliases:
		Default    = NextUpdate,

		NextFrame  = NextUpdate,
		UntilWoken = UntilWake,
		Repeat     = Restart,
		Forever    = Restart,

		next_frame  = NextFrame,
		next_update = NextUpdate,

		until_wake  = UntilWake,
		until_woken = UntilWoken,

		restart     = Restart,
		repeat      = Repeat,
		forever     = Forever,
		
		complete    = Complete,
	};
}