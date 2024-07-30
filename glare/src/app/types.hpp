#pragma once

#include <cstdint>

namespace app
{
	using StepCounter = std::uint64_t;
	using FrameCounter = StepCounter;

	using UpdateRate = std::uint32_t;

	using Milliseconds = std::int64_t; // std::uint64_t;
	using Microseconds = std::int64_t; // std::uint64_t;
}