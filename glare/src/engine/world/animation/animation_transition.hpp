#pragma once

namespace engine
{
	struct AnimationTransition
	{
		// The current time elapsed for this transition.
		float transition_time   = 0.0f;

		// The length of this transition in seconds.
		float transition_length = 1.0f;

		// The rate at which `transition_time` approaches `transition_length`.
		float transition_rate   = 1.0f; // ((1.0f / 60.0f) * 10.0f);

		// If enabled, the source animation will be updated during the transition.
		bool animate_source      : 1 = true;

		// If enabled, the destination animation will be updated during the transition.
		bool animate_destination : 1 = false;

		// Retrieves the current time elapsed for this transition.
		inline float get_transition_time() const
		{
			return this->transition_time;
		}

		// Retrieves the length of this transition in seconds.
		inline float get_transition_length() const
		{
			return this->transition_length;
		}

		// Retrieves the rate at which `transition_time` approaches `transition_length`.
		inline float get_transition_rate() const
		{
			return this->transition_rate;
		}

		// Assigns the rate of transition.
		// (i.e. how quickly `transition_time` approaches `transition_length`)
		inline void set_transition_rate(float transition_rate)
		{
			this->transition_rate = transition_rate;
		}

		// Retrieves whether the source animation will continue updating during the transition.
		inline bool continue_animating_source() const
		{
			return this->animate_source;
		}

		// Retrieves whether the destination animation will continue updating during the transition.
		inline bool continue_animating_destination() const
		{
			return this->animate_destination;
		}

		// This reports true if both the 'source' and 'destination'
		// animations are to be updated during the transition.
		inline bool simultaneous() const
		{
			return (continue_animating_source() && continue_animating_destination());
		}

		inline float length() const
		{
			return transition_length;
		}

		inline bool empty() const
		{
			return (length() <= 0.0f);
		}

		inline explicit operator bool() const
		{
			return (!empty());
		}
	};
}