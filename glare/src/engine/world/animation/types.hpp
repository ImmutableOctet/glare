#pragma once

#include <engine/types.hpp>

namespace engine
{
	using FrameIndex = std::uint32_t;
	using FrameSliceSize = FrameIndex;

	inline static constexpr FrameIndex INVALID_FRAME_INDEX = {};
}