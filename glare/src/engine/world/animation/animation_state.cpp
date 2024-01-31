#include "animation_state.hpp"

#include <math/common.hpp>
#include <math/oscillate.hpp>

#include <cmath>

namespace engine
{
	float AnimationState::get_time() const
	{
		return time;
	}

	float AnimationState::get_rate() const
	{
		return rate;
	}

	bool AnimationState::is_playing() const
	{
		return this->playing;
	}

	bool AnimationState::paused() const
	{
		return (!is_playing());
	}

	bool AnimationState::stopped() const
	{
		return (paused() && (time <= 0.0f));
	}

	bool AnimationState::ended() const
	{
		return (paused() && (time >= 1.0f));
	}

	bool AnimationState::can_end() const
	{
		return ((!repeat) && (!oscillate)); // && (rate != 0.0f)
	}

	bool AnimationState::can_be_applied() const
	{
		return (!stopped());
	}

	bool AnimationState::play()
	{
		this->playing = true;

		return true;
	}

	bool AnimationState::play(float rate)
	{
		if (play())
		{
			this->rate = rate;

			return true;
		}

		return false;
	}

	bool AnimationState::play_at(float time)
	{
		if (play())
		{
			this->time = time;

			return true;
		}

		return false;
	}

	bool AnimationState::play_at(float time, float rate)
	{
		if (play_at(time))
		{
			this->rate = rate;

			return true;
		}

		return false;
	}

	bool AnimationState::pause()
	{
		this->playing = false;

		return true;
	}

	bool AnimationState::pause_at(float time)
	{
		if (pause())
		{
			this->time = time;

			return true;
		}

		return false;
	}

	bool AnimationState::toggle()
	{
		if (playing)
		{
			return pause();
		}
		else
		{
			return play();
		}
	}

	bool AnimationState::stop()
	{
		if (pause())
		{
			this->time = 0.0f;

			return true;
		}

		return false;
	}

	float AnimationState::set_time(float time)
	{
		if (repeat)
		{
			const auto rounded_time = std::fmod(time, 1.0f);

			if (time < 0.0f)
			{
				this->time = (1.0f + rounded_time);
			}
			else
			{
				this->time = rounded_time;
			}
		}
		else if (oscillate)
		{
			this->time = math::oscillate(time);
		}
		else if (time >= 1.0f)
		{
			this->time = 1.0f;

			if (can_end())
			{
				pause();
			}
		}
		else if (time < 0.0f)
		{
			this->time = 0.0f;
		}
		/*
		else
		{
			this->time = math::clamp(time, 0.0f, 1.0f);
		}
		*/

		return this->time;
	}

	float AnimationState::step(float delta)
	{
		if (paused())
		{
			return get_time();
		}

		const auto time_step = (get_rate() * delta);

		return skip(time_step);
	}

	float AnimationState::skip(float distance)
	{
		return set_time((time + distance));
	}
}