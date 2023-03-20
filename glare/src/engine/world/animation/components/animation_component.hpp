#pragma once

#include <engine/types.hpp>
#include <engine/resource_manager/animation_data.hpp>

#include <graphics/vertex.hpp>

#include <optional>

namespace engine
{
	class AnimationSystem;

	struct AnimationComponent
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
				inline TransitionState(float duration, float prev_anim_time, float elapsed=0.0f)
					: duration(duration), prev_time(prev_anim_time), elapsed(elapsed) {}

				float duration;
				float prev_time;
				float elapsed;
			};

			friend AnimationSystem;

			static constexpr auto MAX_CHANNELS = graphics::VERTEX_MAX_BONE_INFLUENCE;
			static constexpr unsigned int MAX_BONES = 128; // 16; // 48;

			static const Animation* resolve_animation(const std::shared_ptr<AnimationData>& animations, AnimationID id);
			static std::optional<AnimationID> resolve_animation_id(const std::shared_ptr<AnimationData>& animations, const Animation* animation);

			AnimationComponent() = default;

			AnimationComponent(float rate);

			inline AnimationComponent(const std::shared_ptr<AnimationData>& animations, AnimationID current_animation={}, float rate=1.0f, float time=0.0f)
				: AnimationComponent(animations, resolve_animation(animations, current_animation), rate, time) {}

			// Animation rate multiplier.
			float rate = 1.0f;

			// Current position/floating frame-position in the animation.
			float time = 0.0f;

			// Buffer containing the last updated state of each bone.
			Matrices pose; // bone_matrices;
		protected:
			std::shared_ptr<AnimationData> animations;

			const Animation* current_animation = nullptr;

			// This indicates the previous animation.
			const Animation* prev_animation = nullptr;

			// This is used to track animation changes between updates.
			// (Mainly used internally to ensure `OnAnimationChange` only fires once, and on the frame the change occurs)
			const Animation* last_known_animation = nullptr;

			State state = State::Play;

			std::optional<TransitionState> transition_state = std::nullopt;
			
			AnimationComponent(const std::shared_ptr<AnimationData>& animations, const Animation* current_animation=nullptr, float rate=1.0f, float time=0.0f);

			inline AnimationComponent& set_animation(const Animation* animation)
			{
				current_animation = animation;

				return *this;
			}

			inline std::optional<AnimationID> resolve_id(const Animation* animation) const
			{
				return resolve_animation_id(this->animations, animation);
			}
		public:
			inline const Animation* get_animation(AnimationID id) const
			{
				return resolve_animation(animations, id); //animations->animations[id];
			}

			inline std::optional<AnimationID> get_animation_id() const
			{
				return resolve_id(get_current_animation());
			}

			inline AnimationComponent& set_animation_id(AnimationID id)
			{
				const auto* animation = get_animation(id);

				// TODO: Determine if a null-check still makes sense here.
				if (!animation)
				{
					return *this;
				}

				return set_animation(animation);
			}

			// Alias for `set_animation_id`.
			inline AnimationComponent& set_animation(AnimationID id)
			{
				return set_animation_id(id);
			}

			inline const auto& get_animations() const
			{
				return animations;
			}

			// TODO: Need to test this part of the public API.
			AnimationComponent& set_animations(const std::shared_ptr<AnimationData>& animations, State new_state=State::Play);

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
}