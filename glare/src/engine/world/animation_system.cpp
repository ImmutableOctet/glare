#include "animation_system.hpp"
#include "animation.hpp"
#include "world.hpp"

#include <engine/events/events.hpp>
#include <engine/relationship.hpp>
#include <engine/resource_manager/resource_manager.hpp>

#include <graphics/animation.hpp>


#include <cmath>

namespace engine
{
	void AnimationSystem::subscribe(Service& svc)
	{
		//world.register_event<...>(*this);
	}

	static std::uint16_t animate_bones(World& world, const math::Matrix& inv_root_matrix, Animator& animator, const Animation& current_animation, Relationship& relationship)
	{
		auto& registry = world.get_registry();

		std::uint16_t bones_animated = 0;

		relationship.enumerate_children(registry, [&bones_animated, &world, &inv_root_matrix, &registry, &animator, &current_animation](Entity child, Relationship& relationship, Entity next_child) -> bool
		{
			auto* bone_detail = registry.try_get<BoneComponent>(child);

			if (!bone_detail)
			{
				return true;
			}

			auto bone_id = bone_detail->ID;

			const Animation::KeySequence* sequence = current_animation.get_sequence(bone_id);

			if (!sequence)
			{
				return true;
			}

			auto local_bone_matrix = sequence->interpolated_matrix(animator.time, false);

			auto bone_matrix = (local_bone_matrix * bone_detail->offset); // true
			//auto bone_matrix = bone_detail->offset;
			//auto bone_matrix = sequence->interpolated_matrix(animator.time);

			//animator.pose[bone_id] = bone_matrix;
			//animator.pose[bone_id] = local_bone_matrix;
			
			{
				auto bone_tform = world.get_transform(child);
				bone_tform.set_local_matrix(local_bone_matrix); // sequence->interpolated_matrix(animator.time, true)
				//bone_tform.set_local_matrix(bone_matrix);

				//animator.pose[bone_id] = glm::identity<glm::mat4>();
				//animator.pose[bone_id] = bone_matrix;

				//animator.pose[bone_id] = (bone_tform.get_matrix() * bone_detail->offset);
				//animator.pose[bone_id] = (bone_tform.get_local_matrix() * bone_detail->offset * inv_root_matrix);

				//animator.pose[bone_id] = bone_tform.get_matrix();

				animator.pose[bone_id] = (inv_root_matrix * bone_tform.get_matrix() * bone_detail->offset);
				//animator.pose[bone_id] = (bone_tform.get_matrix() * bone_detail->offset);
				//animator.pose[bone_id] = bone_detail->offset;
			}

			// Get current bone state (matrix) based on ID, time, and data found in 'current_animation'.
			// Update bone entity's transform in-engine to correspond to new matrix.
			// Log matrix results according to bone ID in array or uniform buffer held in the Animator component.
			// Check against Animator component's existence in render loop,
			// bind already correct shader (see resource manager and co.),
			// upload bone matrices to shader
			// update vertex position based on matrices, weights and indices.
			// ???
			// PROFIT

			auto* bone_rel = registry.try_get<Relationship>(child);

			if (bone_rel)
			{
				bones_animated += animate_bones(world, inv_root_matrix, animator, current_animation, *bone_rel);
			}

			return true;
		}, true);

		return bones_animated;
	}

	static std::uint16_t animate(World& world, Entity entity, Animator& animator, const Animation& current_animation, Relationship& relationship, TransformComponent& tform_comp)
	{
		// Retrieve the inverse world-space matrix of the root entity.
		math::Matrix inv_root_matrix;

		{
			auto tform = Transform(world.get_registry(), entity, relationship, tform_comp);

			inv_root_matrix = tform.get_inverse_matrix();
		}

		return animate_bones(world, inv_root_matrix, animator, current_animation, relationship);
	}

	void AnimationSystem::update(World& world, float delta_time)
	{
		return;

		auto& registry = world.get_registry();

		// Apply motion (gravity, velocity, deceleration, etc.):
		registry.view<Animator, Relationship, TransformComponent>().each([&](auto entity, Animator& animator, Relationship& relationship, TransformComponent& tform_comp)
		{
			if (!animator)
			{
				return;
			}

			// Indicates whether the current animation will advance.
			bool frame_advance = true;

			// Determine if the current animation has changed.
			if (animator.current_animation != animator.last_known_animation)
			{
				// Establish transition period between animations:
				float transition_length = 0.0f;

				if (animator.current_animation)
				{
					// NOTE: We could add an additional check here for foreign animations, but for now we'll assume the same transition behavior applies.
					transition_length = animator.animations->get_transition(animator.last_known_animation->id, animator.current_animation->id);

					if (transition_length > 0.0f)
					{
						animator.transition_state = { transition_length, animator.time };
					}
				}

				world.queue_event(OnAnimationChange { entity, &animator, animator.current_animation, animator.prev_animation, transition_length });

				// Keep track of the last known and previous animations:
				animator.prev_animation = animator.last_known_animation;
				animator.last_known_animation = animator.current_animation;

				// Start the newly assigned animation at the first frame.
				animator.time = 0.0f;

				// We want to start displaying the new animation on the first frame,
				// so we don't need to advance anything from here.
				frame_advance = false;
			}

			const Animation& current_animation = *animator.current_animation;

			auto bones_changed = animate(world, entity, animator, current_animation, relationship, tform_comp);

			if (frame_advance)
			{
				if (animator.state != Animator::State::Pause)
				{
					auto prev_time = animator.time;

					//animator.time = std::fmod((animator.time + ((current_animation.rate * animator.rate) * delta_time)), current_animation.duration);
			
					animator.time += ((current_animation.rate * animator.rate) * delta_time);

					if (animator.time >= current_animation.duration)
					{
						animator.time = 0.0f;
						//animator.time = std::fmod(animator.time, current_animation.duration);
						//animator.time -= current_animation.duration;

						world.event(OnAnimationComplete { entity, &animator, &current_animation });
					}

					world.queue_event(OnAnimationFrame { entity, animator.time, prev_time, &animator, animator.current_animation, bones_changed });
				}
			}
		});
	}
}