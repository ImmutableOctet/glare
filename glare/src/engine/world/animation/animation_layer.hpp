#pragma once

#include "types.hpp"

#include "animation_state.hpp"

namespace engine
{
	struct AnimationLayerData
	{
		AnimationID    name  = {};
		AnimationSlice area  = {};
		AnimationState state = {};

		inline explicit operator bool() const
		{
			return
			(
				static_cast<bool>(name)
				&&
				static_cast<bool>(area)
				&&
				static_cast<bool>(state)
			);
		}
	};

	struct AnimationLayer
	{
		using LayerData = AnimationLayerData;

		// The state of the animation currently playing.
		LayerData current;

		// The state of the animation played prior to `current`.
		LayerData previous;

		// Returns true if `current` reports that it is playing.
		inline bool playing() const
		{
			return current.state.is_playing();
		}

		inline bool paused() const
		{
			return current.state.paused();
		}

		inline bool stopped() const
		{
			return current.state.stopped();
		}

		inline const AnimationState& get_state() const
		{
			return current.state;
		}

		inline AnimationState& get_state()
		{
			return current.state;
		}

		inline bool play(AnimationID name, AnimationSlice area)
		{
			apply
			(
				LayerData
				{
					.name  = name,
					.area  = area,

					.state = AnimationState {}
				}
			);

			return current.state.play();
		}

		inline bool pause()
		{
			return current.state.pause();
		}

		inline bool stop()
		{
			return current.state.stop();
		}

		inline LayerData& apply(LayerData&& data)
		{
			previous = std::move(current);
			current  = std::move(data);

			return current;
		}
	};

	// Computes a bitmask for a given `layer_index`.
	// (Enables the bit corresponding to the index specified)
	// 
	// The `layer_index` argument must be smaller than `MAX_ANIMATION_LAYERS`.
	AnimationLayerMask compute_animation_layer_mask(AnimationLayerIndex layer_index);

	// Computes a bitmask with bits enabled for every layer up to `layer_count`.
	// 
	// The `layer_count` argument must be less than or equal to `MAX_ANIMATION_LAYERS`.
	AnimationLayerMask compute_mask_for_animation_layers(AnimationLayerCount layer_count);
}