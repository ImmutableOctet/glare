#include "animation_system.hpp"

#include "animation_events.hpp"
#include "animation_state.hpp"
#include "animation_repository.hpp"

#include "components/animation_component.hpp"
#include "components/animation_transition_component.hpp"
#include "components/bone_component.hpp"
#include "components/skeletal_component.hpp"
#include "components/skeletal_pose_component.hpp"

#include <engine/entity/entity_descriptor.hpp>
#include <engine/entity/components/instance_component.hpp>

#include <engine/components/relationship_component.hpp>
#include <engine/components/forwarding_component.hpp>
#include <engine/components/model_component.hpp>

#include <engine/resource_manager/animation_data.hpp>

#include <engine/meta/hash.hpp>
#include <engine/world/world.hpp>
#include <engine/resource_manager/resource_manager.hpp>
#include <engine/events.hpp>

#include <cmath>
#include <cstdint>

namespace engine
{
	AnimationSystem::AnimationSystem(World& world)
		: WorldSystem(world)
	{}

	void AnimationSystem::on_subscribe(World& world)
	{
		world.register_event<OnParentChanged, &AnimationSystem::on_parent_changed>(*this);

		auto& registry = world.get_registry();

		//registry.on_construct<AnimationComponent>().connect<&AnimationSystem::on_animation_attached>(*this);
		//registry.on_destroy<AnimationComponent>().connect<&AnimationSystem::on_animation_detached>(*this);
		//registry.on_update<AnimationComponent>().connect<&AnimationSystem::on_animation_changed>(*this);
	}

	void AnimationSystem::on_unsubscribe(World& world)
	{
		auto& registry = world.get_registry();

		//registry.on_construct<AnimationComponent>().disconnect(*this);
		//registry.on_destroy<AnimationComponent>().disconnect(*this);
		//registry.on_update<AnimationComponent>().disconnect(*this);
	}

	void AnimationSystem::update_states(World& world, float delta_time)
	{
		auto& registry = world.get_registry();

		registry.view<AnimationComponent>().each
		(
			[&](Entity entity, AnimationComponent& animation)
			{
				for (auto& layer : animation.layers)
				{
					auto& current = layer.current;

					if (current)
					{
						if (current.state.is_playing())
						{
							current.state.step(delta_time);
						}
					}

					auto& previous = layer.previous;

					if (previous)
					{
						if (current.state.is_playing())
						{
							previous.state.step(delta_time);
						}
					}
				}
			}
		);
	}

	std::size_t AnimationSystem::update_bones(World& world)
	{
		auto& registry = world.get_registry();

		auto bones_updated = std::size_t {};

		registry.view<RelationshipComponent, TransformComponent, AnimationComponent, SkeletalPoseComponent>().each
		(
			[&](Entity entity, RelationshipComponent& relationship, TransformComponent& transform_component, AnimationComponent& animation, SkeletalPoseComponent& skeletal_component)
			{
				auto& layer = animation.get_primary_layer();

				if (!layer.current)
				{
					return;
				}

				if (const auto asset_data = get_data_from_asset(registry, entity))
				{
					if (const auto& frame_data = asset_data->frames)
					{
						auto& current = layer.current;

						const auto& area = current.area;

						const auto frames_begin = static_cast<float>(area.from);
						const auto frames_end   = static_cast<float>(area.to);

						const auto frame_count  = area.size();

						// Normalized linear time elapsed along animation slice. (0.0 to 1.0)
						const auto normalized_time_elapsed = current.state.get_time();

						// Frame number/offset relative to `frames_begin`.
						const auto local_frame_number = (static_cast<float>(frame_count) * normalized_time_elapsed);

						// Frame number relative to all animations.
						const auto current_frame = (frames_begin + local_frame_number);

						// Retrieve the inverse world-space matrix of the root entity.
						auto inverse_root_matrix = math::Matrix{};

						{
							auto root_transform = Transform(registry, entity, relationship, transform_component);

							inverse_root_matrix = root_transform.get_inverse_matrix();
						}

						relationship.enumerate_children
						(
							registry,

							[&](Entity child, const RelationshipComponent& child_relationship, Entity next_child) -> bool
							{
								if (const auto* bone_detail = registry.try_get<BoneComponent>(child))
								{
									const auto& bone_index = bone_detail->bone_index;

									if (const auto key_sequence = frame_data.get_sequence(bone_index))
									{
										auto bone_transform = world.get_transform(child);

										const auto local_bone_matrix = key_sequence->interpolated_matrix(current_frame, frames_begin, frames_end);

										bone_transform.set_local_matrix(local_bone_matrix);

										auto bone_offset = math::Matrix {};

										if (const auto bone = asset_data->skeleton.get_bone_by_index(bone_index))
										{
											bone_offset = bone->offset;
										}

										skeletal_component.pose.bone_matrices[bone_index] = (inverse_root_matrix * bone_transform.get_matrix() * bone_offset);

										bones_updated++;
									}
								}

								return true;
							},

							true
						);
					}
				}

				/*
				world.queue_event(OnAnimationFrame { entity, animator.time, prev_time, &animator, animator.current_animation, bones_changed });

				if (prev_time > current_time)
				{
					//world.queue_event<OnAnimationRepeat>(...);
				}
				else if (current_animation.ended())
				{

				}
				*/
			}
		);

		return bones_updated;
	}

	void AnimationSystem::on_update(World& world, float delta_time)
	{
		auto& registry = world.get_registry();

		update_states(world, delta_time);
		update_bones(world);
	}

	AnimationSlice AnimationSystem::get_slice(Registry& registry, Entity entity, AnimationID animation_id) const
	{
		if (auto slice_from_asset = get_slice_from_asset(registry, entity, animation_id))
		{
			return slice_from_asset;
		}

		if (const auto slice_from_repository = get_slice_from_repository(registry, entity, animation_id))
		{
			return slice_from_repository;
		}

		return {};
	}

	AnimationSlice AnimationSystem::get_slice_from_asset(Registry& registry, Entity entity, AnimationID animation_id) const
	{
		if (const auto slices_from_asset = get_slices_from_asset(registry, entity))
		{
			return get_slice_from_asset(*slices_from_asset, animation_id);
		}

		return {};
	}

	AnimationSlice AnimationSystem::get_slice_from_asset(const AnimationData::AnimationContainer& animation_slices, AnimationID animation_id) const
	{
		if (const auto animation_it = animation_slices.find(animation_id); animation_it != animation_slices.end())
		{
			return animation_it->second;
		}

		return {};
	}

	AnimationSlice AnimationSystem::get_slice_from_repository(Registry& registry, Entity entity, AnimationID animation_id) const
	{
		if (const auto animation_repository = get_repository(registry, entity))
		{
			return get_slice_from_repository(*animation_repository, animation_id);
		}

		return {};
	}

	AnimationSlice AnimationSystem::get_slice_from_repository(const AnimationRepository& animation_repository, AnimationID animation_id) const
	{
		if (const auto stored_slice = animation_repository.get_slice(animation_id))
		{
			return *stored_slice;
		}

		return {};
	}

	const AnimationSequence* AnimationSystem::get_sequence(Registry& registry, Entity entity, AnimationID animation_id) const
	{
		if (const auto animation_repository = get_repository(registry, entity))
		{
			return get_sequence(*animation_repository, animation_id);
		}

		return {};
	}

	const AnimationSequence* AnimationSystem::get_sequence(const AnimationRepository& animation_repository, AnimationID animation_id) const
	{
		return animation_repository.get_sequence(animation_id);
	}

	std::size_t AnimationSystem::play(Registry& registry, Entity entity, AnimationID animation_id, AnimationLayerMask animation_layer)
	{
		if (const auto repository = get_repository(registry, entity))
		{
			if (const auto animation_sequence = get_sequence(*repository, animation_id))
			{
				//return begin_playing_sequence();
			}

			if (const auto animation_slice = get_slice_from_repository(*repository, animation_id))
			{
				return play_slice(registry, entity, animation_id, animation_slice, animation_layer);
			}
		}
		else
		{
			if (const auto animation_slice = get_slice_from_asset(registry, entity, animation_id))
			{
				return play_slice(registry, entity, animation_id, animation_slice, animation_layer);
			}
		}

		return {};
	}

	std::size_t AnimationSystem::play(Registry& registry, Entity entity, std::string_view animation_name, AnimationLayerMask animation_layer)
	{
		return play(registry, entity, hash(animation_name), animation_layer);
	}

	const Skeleton* AnimationSystem::get_skeleton(Registry& registry, Entity entity) const
	{
		if (const auto asset_data = get_data_from_asset(registry, entity))
		{
			return &asset_data->skeleton;
		}

		return {};
	}

	const SkeletalFrameData* AnimationSystem::get_frame_data(Registry& registry, Entity entity) const
	{
		if (const auto asset_data = get_data_from_asset(registry, entity))
		{
			return &asset_data->frames;
		}

		return {};
	}

	std::size_t AnimationSystem::play_slice
	(
		Registry& registry, Entity entity,
		AnimationID animation_id, AnimationSlice animation_slice,
		AnimationLayerMask animation_layer
	)
	{
		auto& animation_component = registry.get_or_emplace<AnimationComponent>(entity);

		return animation_component.play(animation_id, animation_slice, animation_layer);
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
			if (!attach_skeleton(registry, parent_changed.to_parent, parent_changed.entity)) // In the event of failure:
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
		relationship->enumerate_children
		(
			registry,
			[&](Entity child, RelationshipComponent& relationship, Entity next_child)
			{
				auto* bone = registry.try_get<BoneComponent>(child);

				if (!bone)
					return true;

				set_bone_skeleton(registry, child, *bone, parent_changed.to_parent);

				return true;
			},
			true
		);
	}

	const AnimationRepository* AnimationSystem::get_repository(Registry& registry, Entity entity) const
	{
		auto instance_comp = registry.try_get<InstanceComponent>(entity);

		if (!instance_comp)
		{
			return {};
		}

		const auto& descriptor = instance_comp->get_descriptor();

		return &(descriptor.animations);
	}

	const AnimationData::AnimationContainer* AnimationSystem::get_slices_from_asset(Registry& registry, Entity entity) const
	{
		if (const auto animation_data = get_data_from_asset(registry, entity))
		{
			return &(animation_data->animations);
		}

		return {};
	}

	const AnimationData* AnimationSystem::get_data_from_asset(Registry& registry, Entity entity) const
	{
		const auto model_comp = registry.try_get<ModelComponent>(entity);

		if (!model_comp)
		{
			return {};
		}

		const auto& world = get_world();
		const auto& resource_manager = world.get_resource_manager();

		return resource_manager.peek_animation_data(model_comp->model);
	}

	/*
	void AnimationSystem::on_animation_attached(Registry& registry, Entity entity)
	{
	}

	void AnimationSystem::on_animation_detached(Registry& registry, Entity entity)
	{
	}

	void AnimationSystem::on_animation_changed(Registry& registry, Entity entity)
	{
		auto& animation_comp = registry.get<AnimationComponent>(entity);

		if (animation_comp.current == animation_comp.previous)
		{
			return;
		}

		auto& animation_state_comp = registry.get_or_emplace<AnimationStateComponent>(entity);

		resolve_animation
		(
			registry, entity,

			animation_comp,
			animation_state_comp,

			animation_comp.current
		);
	}
	*/
}