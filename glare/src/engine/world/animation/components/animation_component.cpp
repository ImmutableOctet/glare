#include "animation_component.hpp"

#include <engine/resource_manager/resource_manager.hpp>
#include <graphics/animation.hpp>

#include <algorithm>

namespace engine
{
	const Animation* Animator::resolve_animation(const std::shared_ptr<AnimationData>& animations, AnimationID id)
	{
		//assert(animations);

		if (!animations)
		{
			return nullptr;
		}

		//return &(animations->animations.at(id));
		return &animations->animations[id];
	}

	std::optional<AnimationID> Animator::resolve_animation_id(const std::shared_ptr<AnimationData>& animations, const Animation* animation)
	{
		if ((!animations) || (!animation))
		{
			return std::nullopt;
		}

		// NOTE: Animation IDs are currently stored in `Animation` objects. This may change later.
		return animation->id;

		/*
		// Alternative implementation:
		const auto& animation_container = animations->animations;

		AnimationID idx = 0;

		for (const auto& element : animation_container)
		{
			//if (element.id == animation->id)
			if (&element == animation)
			{
				//return element.id;
				return idx;
			}

			idx++;
		}

		return std::nullopt;
		*/
	}

	Animator::Animator(float rate)
		: rate(rate) {}

	Animator::Animator
	(
		const std::shared_ptr<AnimationData>& animations,
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

	Animator& Animator::set_animations(const std::shared_ptr<AnimationData>& animations, State new_state)
	{
		this->animations = animations;

		this->current_animation = nullptr;
		this->prev_animation = nullptr;

		//this->transition_state = std::nullopt;

		state = new_state;

		return *this;
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