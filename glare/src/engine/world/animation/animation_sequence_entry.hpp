#pragma once

#include "animation_transition.hpp"

#include <engine/types.hpp>

namespace engine
{
	struct AnimationSequenceEntry
	{
		// An identifier used to resolve the next animation in the sequence.
		AnimationID animation = {};

		// The rate at which `animation` will be played.
		float rate = engine::DEFAULT_RATE; // (1.0f / 60.0f);

		// Specifies the transition to be applied when switching from
		// `animation` to the next animation in the sequence.
		AnimationTransition transition = {};

		// Returns true if `animation` is specified.
		inline bool has_animation() const
		{
			return static_cast<bool>(animation);
		}

		// Returns true if `transition` is non-empty.
		inline bool has_transition() const
		{
			return static_cast<bool>(transition);
		}

		// Equivalent to `has_animation`.
		inline explicit operator bool() const
		{
			return has_animation();
		}
	};
}