#pragma once

#include <engine/types.hpp>
#include <graphics/vertex.hpp>

#include <optional>

namespace engine
{
	class AnimationSystem;

	// Main animation component type.
	struct Animator
	{
		public:
			using Matrices = std::vector<math::Matrix>;

			enum class State : std::uint8_t
			{
				Play,
				Pause,
				//Transition, // Not needed; see transition_state.
			};

			struct TransitionState
			{
				inline TransitionState(float duration, float prev_anim_time)
					: duration(duration), prev_time(prev_anim_time) {}

				float duration;
				float prev_time;
				float elapsed = 0.0f;
			};

			friend AnimationSystem;

			static constexpr auto MAX_CHANNELS = graphics::VERTEX_MAX_BONE_INFLUENCE;
			static constexpr unsigned int MAX_BONES = 128; // 16; // 48;

			static const Animation& get_animation(pass_ref<AnimationData> animations, AnimationID id);

			inline Animator(pass_ref<AnimationData> animations, AnimationID current_animation={}, float rate=1.0f, float time=0.0f)
				: Animator(animations, &(get_animation(animations, current_animation)), rate, time) {}

			inline const Animation& get_animation(AnimationID id) const
			{
				return get_animation(animations, id); //animations->animations[id];
			}

			// Animation rate multiplier.
			float rate;

			// Current position/floating frame-position in the animation.
			float time;

			// Buffer containing the last updated state of each bone.
			Matrices pose; // bone_matrices;
		protected:
			ref<AnimationData> animations;

			const Animation* current_animation = nullptr;

			// This indicates the previous animation.
			const Animation* prev_animation = nullptr;

			// This is used to track animation changes between updates.
			// (Mainly used internally to ensure `OnAnimationChange` only fires once, and on the frame the change occurs)
			const Animation* last_known_animation = nullptr;

			State state = State::Play;

			std::optional<TransitionState> transition_state = std::nullopt;
			
			Animator(pass_ref<AnimationData> animations, const Animation* current_animation=nullptr, float rate=1.0f, float time=0.0f);

			inline Animator& set_animation(const Animation* animation)
			{
				current_animation = animation;

				return *this;
			}
		public:
			inline Animator& set_animation(AnimationID id)
			{
				const auto& animation = get_animation(id);

				return set_animation(&animation);
			}

			inline const auto& get_animations() const
			{
				return animations;
			}

			inline const Animation* get_current_animation() const { return current_animation; }
			inline const Animation* get_prev_animation() const { return prev_animation; }

			inline State get_state() const { return state; }

			inline bool paused() const { return (state == State::Pause); }
			inline bool playing() const { return !paused(); }
			inline bool transitioning() const { return transition_state.has_value(); }

			inline bool animated() const { return ((current_animation) && (playing())); }
			inline explicit operator bool() const { return animated(); }

			inline const std::vector<math::Matrix>& get_pose() const
			{
				return pose;
			}

			inline std::size_t pose_size() const { return pose.size(); }

			void play();
			void pause();
			bool toggle();
	};

	using AnimationComponent = Animator;
}