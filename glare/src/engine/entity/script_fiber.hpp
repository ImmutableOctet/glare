#pragma once

#include "script_fiber_response.hpp"

#include <engine/fiber.hpp>

namespace engine
{
	using ScriptFiber = Fiber<ScriptFiberResponse>;

	using EntityThreadFiber            = ScriptFiber;
	using EntityThreadFiberResponse    = ScriptFiberResponse;
	using EntityThreadControlFlowToken = ScriptControlFlowToken;
}