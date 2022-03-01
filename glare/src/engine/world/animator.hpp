#pragma once

#include <engine/types.hpp>
#include <graphics/vertex.hpp>

namespace engine
{
	class AnimationSystem;

	struct Animator
	{
		public:
			friend AnimationSystem;

			static constexpr auto MAX_CHANNELS = graphics::VERTEX_MAX_BONE_INFLUENCE;
			static constexpr unsigned int MAX_BONES = 128; // 16; // 48;

			Animator(pass_ref<AnimationData> animations, const Animation* current_animation=nullptr, float rate=1.0f, float time=0.0f);

			ref<AnimationData> animations;

			// Animation rate multiplier.
			float rate;

			// Current position/floating frame-position in the animation.
			float time;

			// Buffer containing the last updated state of each bone.
			std::vector<math::Matrix> pose; // bone_matrices;
		protected:
			const Animation* current_animation = nullptr;
			const Animation* prev_animation = nullptr;
			const Animation* last_known_animation = nullptr;
		public:
			inline Animator& set_animation(const Animation* animation)
			{
				current_animation = animation;

				return *this;
			}

			inline const Animation* get_current_animation() const { return current_animation; }
			inline const Animation* get_prev_animation() const { return prev_animation; }

			inline bool animated() const { return current_animation; }
			inline explicit operator bool() const { return animated(); }

			inline const std::vector<math::Matrix>& get_pose() const
			{
				return pose;
			}

			inline std::size_t pose_size() const { return pose.size(); }
	};
}