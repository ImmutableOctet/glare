#pragma once

#include "types.hpp"

#include <cassert>

namespace engine
{
	// Describes a range of frame indices, used to compose an animation.
	struct AnimationSlice
	{
		FrameIndex from = {};
		FrameIndex to   = {};

		inline FrameSliceSize size() const
		{
			assert(to > from);

			return (to - from);
		}

		inline bool empty() const
		{
			return (size() == 0);
		}

		inline explicit operator bool() const
		{
			return (!empty());
		}

		inline AnimationSlice value_or(const AnimationSlice& slice) const
		{
			if (empty())
			{
				return slice;
			}

			return *this;
		}
	};
}