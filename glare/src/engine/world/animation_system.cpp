#include "animation_system.hpp"
#include "animation.hpp"
#include "world.hpp"

#include <engine/events/events.hpp>
#include <engine/relationship.hpp>
#include <graphics/animation.hpp>

#include <cmath>

namespace engine
{
	void AnimationSystem::subscribe(Service& svc)
	{
		//world.register_event<...>(*this);
	}

	static void animate_bones(World& world, const math::Matrix& inv_root_matrix, Animator& animator, const Animation& current_animation, Relationship& relationship)
	{
		auto& registry = world.get_registry();

		relationship.enumerate_children(registry, [&world, &inv_root_matrix, &registry, &animator, &current_animation](Entity child, Relationship& relationship, Entity next_child) -> bool
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
				animate_bones(world, inv_root_matrix, animator, current_animation, *bone_rel);
			}

			return true;
		}, true);
	}

	void AnimationSystem::update(World& world, float delta_time)
	{
		//return;

		auto& registry = world.get_registry();

		// Apply motion (gravity, velocity, deceleration, etc.):
		registry.view<Animator, Relationship, TransformComponent>().each([&](auto entity, Animator& animator, Relationship& relationship, TransformComponent& tform_comp)
		{
			if (!animator)
			{
				return;
			}

			if (animator.current_animation != animator.last_known_animation)
			{
				world.queue_event(OnAnimationChange { entity, &animator, animator.current_animation, animator.prev_animation });

				animator.prev_animation = animator.last_known_animation;
				animator.last_known_animation = animator.current_animation;
			}

			const Animation& current_animation = *animator.current_animation;

			math::Matrix inv_root_matrix;

			{
				auto tform = Transform(registry, entity, relationship, tform_comp);

				inv_root_matrix = tform.get_inverse_matrix();
			}

			animate_bones(world, inv_root_matrix, animator, current_animation, relationship);

			//if (current_animation.duration > 1.0f) ...
			
			//animator.time = std::fmod((animator.time + ((current_animation.rate * animator.rate) * delta_time)), current_animation.duration);

			auto prev_time = animator.time;
			
			animator.time += ((current_animation.rate * animator.rate) * delta_time) * 0.1;

			if (animator.time >= current_animation.duration)
			{
				animator.time = 0.0f;
				//animator.time = std::fmod(animator.time, current_animation.duration);
				//animator.time -= current_animation.duration;

				world.event(OnAnimationComplete { entity, &animator, &current_animation });
			}

			world.queue_event(OnAnimationFrame { entity, animator.time, prev_time, &animator, animator.current_animation });
		});
	}
}