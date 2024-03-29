#pragma once

#include "control_flow_token.hpp"

#include <util/fiber.hpp>

namespace engine
{
	template <typename T = void>
	using Fiber = util::fiber<T>;

	using AnyFiber = Fiber<void>;
	using ControlFlowFiber = Fiber<ControlFlowToken>;
}