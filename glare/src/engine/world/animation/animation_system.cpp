#include "animation_system.hpp"
#include "animation_events.hpp"

#include "components/animation_component.hpp"
#include "components/bone_component.hpp"
#include "components/skeletal_component.hpp"

#include <engine/components/relationship_component.hpp>
#include <engine/components/forwarding_component.hpp>
#include <engine/world/world.hpp>
#include <engine/resource_manager/resource_manager.hpp>
#include <engine/events.hpp>

#include <graphics/animation.hpp>

#include <cmath>

namespace engine
{
	AnimationSystem::AnimationSystem(World& world)
		: WorldSystem(world)
	{}

	void AnimationSystem::on_subscribe(World& world)
	{
		world.register_event<OnParentChanged, &AnimationSystem::on_parent_changed>(*this);
	}

	static std::uint16_t animate_bones(World& world, const math::Matrix& inv_root_matrix, AnimationComponent& animator, const Animation& current_animation, RelationshipComponent& relationship)
	{
		auto& registry = world.get_registry();

		std::uint16_t bones_animated = 0;

		relationship.enumerate_children(registry, [&bones_animated, &world, &inv_root_matrix, &registry, &animator, &current_animation](Entity child, RelationshipComponent& relationship, Entity next_child) -> bool
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
			// Log matrix results according to bone ID in array or uniform buffer held in the AnimationComponent component.
			// Check against AnimationComponent component's existence in render loop,
			// bind already correct shader (see resource manager and co.),
			// upload bone matrices to shader
			// update vertex position based on matrices, weights and indices.
			// ???
			// PROFIT

			auto* bone_rel = registry.try_get<RelationshipComponent>(child);

			if (bone_rel)
			{
				bones_animated += animate_bones(world, inv_root_matrix, animator, current_animation, *bone_rel);
			}

			return true;
		}, true);

		return bones_animated;
	}

	static std::uint16_t animate(World& world, Entity entity, AnimationComponent& animator, const Animation& current_animation, RelationshipComponent& relationship, TransformComponent& tform_comp)
	{
		// Retrieve the inverse world-space matrix of the root entity.
		math::Matrix inv_root_matrix;

		{
			auto tform = Transform(world.get_registry(), entity, relationship, tform_comp);

			inv_root_matrix = tform.get_inverse_matrix();
		}

		return animate_bones(world, inv_root_matrix, animator, current_animation, relationship);
	}

	void AnimationSystem::on_update(World& world, float delta_time)
	{
		return;

		auto& registry = world.get_registry();

		// Apply motion (gravity, velocity, deceleration, etc.):
		registry.view<AnimationComponent, RelationshipComponent, TransformComponent>().each([&](auto entity, AnimationComponent& animator, RelationshipComponent& relationship, TransformComponent& tform_comp)
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
				if (animator.state != AnimationComponent::State::Pause)
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

	void AnimationSystem::set_bone_skeleton(Registry& registry, Entity entity, BoneComponent& bone_component, Entity new_skeleton)
	{
		bone_component.skeleton = new_skeleton;

		// Set the forwarding component to the `new_skeleton` entity.
		registry.emplace_or_replace<ForwardingComponent>(entity, new_skeleton);
	}

	void AnimationSystem::on_parent_changed(const OnParentChanged& parent_changed)
	{
		auto& registry = world.get_registry();

		auto* bone_component = registry.try_get<BoneComponent>(parent_changed.entity);

		if (!bone_component)
		{
			return;
		}
		
		// Check if the previous parent had a skeleton:
		if (parent_changed.from_parent != null)
		{
			if (auto* prev_parent_skeleton = registry.try_get<SkeletalComponent>(parent_changed.from_parent); prev_parent_skeleton)
			{
				// Check if we're the root bone:
				if (prev_parent_skeleton->root_bone == parent_changed.entity)
				{
					// Set the previous parent's root bone to `null`.
					prev_parent_skeleton->root_bone = null;
				}
			}
		}
		
		// If the newly assigned parent entity is a bone, forward its skeleton field:
		if (auto* new_parent_bone_component = registry.try_get<BoneComponent>(parent_changed.to_parent); new_parent_bone_component)
		{
			set_bone_skeleton(registry, parent_changed.entity, *bone_component, new_parent_bone_component->skeleton);
		}
		else // Parent is not a bone:
		{
			// Point our own skeleton field to the new parent/skeleton entity.
			set_bone_skeleton(registry, parent_changed.entity, *bone_component, parent_changed.to_parent);

			// Attach a skeleton to the new parent.
			if (!attach_skeleton(world, parent_changed.to_parent, parent_changed.entity)) // In the event of failure:
			{
				// A skeleton already exists, assign our parent to that skeleton's `root_bone` instead:
				if (auto* skeleton = registry.try_get<SkeletalComponent>(parent_changed.to_parent); skeleton)
				{
					assert(skeleton->root_bone != null);

					world.set_parent(parent_changed.entity, skeleton->root_bone);

					// Don't bother continuing, since we'll have to handle a new `OnParentChanged` event shortly.
					return;
				}
			}
		}

		auto* relationship = registry.try_get<RelationshipComponent>(parent_changed.entity);

		if (!relationship)
		{
			return;
		}
		
		// Update children recursively with our newly established skeleton entity:
		relationship->enumerate_children(registry, [&](Entity child, RelationshipComponent& relationship, Entity next_child)
		{
			auto* bone = registry.try_get<BoneComponent>(child);

			if (!bone)
				return true;

			set_bone_skeleton(registry, child, *bone, parent_changed.to_parent);

			return true;
		}, true);
	}
}