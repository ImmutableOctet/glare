#pragma once

#include "script_fiber_response.hpp"

#include <engine/fiber.hpp>

namespace engine
{
	using ScriptFiber = Fiber<ScriptFiberResponse>;

	using EntityThreadFiberResponse    = ScriptFiberResponse;
}