#include "animator.hpp"

#include <engine/resource_manager/resource_manager.hpp>
#include <graphics/animation.hpp>

namespace engine
{
	const Animation& Animator::get_animation(pass_ref<AnimationData> animations, AnimationID id)
	{
		return animations->animations.at(id); //animations->animations[id];
	}

	Animator::Animator
	(
		pass_ref<AnimationData> animations,
		const Animation* current_animation,
		float rate, float time
	) :
		animations(animations),
		current_animation(current_animation),
		prev_animation(current_animation), // prev_animation(nullptr)
		last_known_animation(current_animation),
		rate(rate), time(time)
	{
		assert(animations);

		auto number_of_bones = animations->skeleton.size();

		/*
		pose.reserve(number_of_bones); // MAX_BONES

		for (decltype(number_of_bones) i = 0; i < number_of_bones; i++)
		{
			const auto* bone = animations->skeleton.get_bone(i);

			math::Matrix bone_matrix;

			if (bone)
			{
				bone_matrix = bone->offset;
			}
			else
			{
				bone_matrix = math::identity_matrix();
			}

			pose.push_back(bone_matrix);
		}
		*/

		pose = Matrices(number_of_bones);
	}

	void Animator::play()
	{
		state = State::Play;
	}

	void Animator::pause()
	{
		state = State::Pause;
	}

	bool Animator::toggle()
	{
		if (playing())
		{
			pause();

			return true;
		}
		else // if (paused())
		{
			play();

			return false;
		}
	}
}