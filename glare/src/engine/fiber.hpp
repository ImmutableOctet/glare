#pragma once

#include <util/fiber.hpp>

namespace engine
{
	template <typename T=void>
	using Fiber = util::fiber<T>;

	using AnyFiber = Fiber<void>;
}