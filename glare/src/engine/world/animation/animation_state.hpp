#pragma once

#include "animation_slice.hpp"

namespace engine
{
	struct AnimationState
	{
		// A normalized value representing a position along the animation `area`.
		// (Clamped from 0.0 to 1.0)
		float time = 0.0f;

		// The rate at which `time` advances along the animation `area`.
		// A negative rate results in reversed playback.
		float rate = engine::DEFAULT_RATE; // (1.0f / 60.0f);

		// If enabled, the animation `time` will progress at `rate` along `area`.
		// If disabled, animation will be paused at the current `time`.
		bool playing   : 1 = true;

		// If enabled, animation will continue upon reaching the end of the animation boundary.
		// 
		// If `oscillate` is enabled when this boundary is reached, the animation direction will be inverted.
		// If `oscillate` is disabled when this boundary is reached, the animation will be restarted from the first frame in the designated area.
		bool repeat    : 1 = true;

		// If enabled, the animation direction will be toggled upon
		// reaching the end of the animation area/slice.
		// 
		// The result is an animation which continually switches between forward
		// and backward playback upon reaching the animation boundary.
		bool oscillate : 1 = false;

		// Retrieves the current normalized `time` of the animation.
		float get_time() const;

		// Retrieves the current rate of animation.
		float get_rate() const;

		// Returns true if this animation is currently playing.
		bool is_playing() const;

		// Returns true if the `playing` field is false.
		// (i.e. the animation is paused on the current frame)
		bool paused() const;

		// Returns true if the animation has been stopped.
		bool stopped() const;

		// Returns true if the animation has ended.
		bool ended() const;

		// Returns true if the animation can end without outside interference.
		bool can_end() const;

		// Returns true if a pose from this animation can be applied.
		bool can_be_applied() const;

		// Begins playing the animation using its current configuration.
		bool play();

		// Begins playing the animation using the `rate` designated.
		bool play(float rate);

		// Begins playing the animation at the `time` specified using the established `rate`.
		bool play_at(float time);

		// Begins playing the animation at the `time` specified using the desired `rate`.
		bool play_at(float time, float rate);

		// Pauses the animation at the current `time`.
		bool pause();

		// Pauses the animation at the `time` specified.
		bool pause_at(float time);

		// This toggles between playing and paused animation states.
		// The return value of this member-function indicates if the animation is now playing.
		bool toggle();

		// Stops the current animation.
		// NOTE: This does not guaranteed an 'unposed' model.
		bool stop();

		// Sets the active time of the animation,
		// clamping, overflowing or underflowing as needed.
		float set_time(float time);

		// Adjusts `time` forward by `rate` multiplied by `delta`.
		float step(float delta=1.0f);

		// Skips forward the normalized `distance` specified.
		float skip(float distance);

		// Equivalent to `can_be_applied`.
		inline explicit operator bool() const
		{
			return can_be_applied();
		}
	};
}