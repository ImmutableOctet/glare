#pragma once

#include <cstdint>

namespace engine
{
	// The targeted number of 'frames' (i.e. updates) per-second.
	inline constexpr std::uint32_t TARGET_FRAMES_PER_SECOND = 60;

	// The targeted number of network updates (i.e. packets) per-second.
	inline constexpr std::uint32_t TARGET_NETWORK_UPDATES_PER_SECOND = TARGET_FRAMES_PER_SECOND; // 64;

	// A fraction of a second, representing the approximate length of a 'frame'.
	inline constexpr double TARGET_FRAME_DELTA_EX = (1.0 / static_cast<double>(TARGET_FRAMES_PER_SECOND));

	// A fraction of a second, representing the approximate length of a 'frame'.
	inline constexpr float TARGET_FRAME_DELTA = static_cast<float>(TARGET_FRAME_DELTA_EX);
}