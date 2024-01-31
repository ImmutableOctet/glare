#pragma once

#include "animation_sequence_entry.hpp"

#include <util/small_vector.hpp>

namespace engine
{
	// Used to describe a higher-level sequence of animations to be played.
	struct AnimationSequence
	{
		using Entry = AnimationSequenceEntry;
		using Container = util::small_vector<Entry, 4>; // 8

		Container animations;

		inline std::size_t size() const
		{
			return animations.size();
		}

		inline bool empty() const
		{
			return animations.empty();
		}

		inline explicit operator bool() const
		{
			return (!empty());
		}
	};
}