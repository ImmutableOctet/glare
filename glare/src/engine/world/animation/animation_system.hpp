#pragma once

#include "types.hpp"
#include "animation_slice.hpp"

#include <engine/resource_manager/animation_data.hpp>
#include <engine/world/world_system.hpp>

#include <string_view>
#include <optional>

namespace engine
{
	class World;
	class Service;

	class AnimationRepository;
	struct AnimationSequence;

	struct OnParentChanged;

	struct AnimationComponent;
	struct BoneComponent;

	class AnimationSystem : public WorldSystem
	{
		public:
			AnimationSystem(World& world);

			void on_subscribe(World& world) override;
			void on_unsubscribe(World& world) override;

			void on_update(World& world, float delta_time) override;

			std::size_t play(Registry& registry, Entity entity, AnimationID animation_id, AnimationLayerMask animation_layer=ANIMATION_LAYER_MASK_NO_LAYER);
			std::size_t play(Registry& registry, Entity entity, std::string_view animation_name, AnimationLayerMask animation_layer=ANIMATION_LAYER_MASK_NO_LAYER);

			const Skeleton* get_skeleton(Registry& registry, Entity entity) const;

			const SkeletalFrameData* get_frame_data(Registry& registry, Entity entity) const;

			AnimationSlice get_slice(Registry& registry, Entity entity, AnimationID animation_id) const;
			
			AnimationSlice get_slice_from_asset(Registry& registry, Entity entity, AnimationID animation_id) const;
			AnimationSlice get_slice_from_asset(const AnimationData::AnimationContainer& animation_slices, AnimationID animation_id) const;
			
			AnimationSlice get_slice_from_repository(Registry& registry, Entity entity, AnimationID animation_id) const;
			AnimationSlice get_slice_from_repository(const AnimationRepository& animation_repository, AnimationID animation_id) const;
			
			const AnimationSequence* get_sequence(Registry& registry, Entity entity, AnimationID animation_id) const;
			const AnimationSequence* get_sequence(const AnimationRepository& animation_repository, AnimationID animation_id) const;

		protected:
			void update_states(World& world, float delta_time);

			std::size_t update_bones(World& world);

			std::size_t play_slice
			(
				Registry& registry, Entity entity,
				AnimationID animation_id, AnimationSlice animation_slice,
				AnimationLayerMask animation_layer=ANIMATION_LAYER_MASK_NO_LAYER
			);

			// Used internally by `on_parent_changed`.
			void set_bone_skeleton(Registry& registry, Entity entity, BoneComponent& bone_component, Entity new_skeleton);

			void on_parent_changed(const OnParentChanged& parent_changed);

			/*
			void on_animation_attached(Registry& registry, Entity entity);
			void on_animation_detached(Registry& registry, Entity entity);
			void on_animation_changed(Registry& registry, Entity entity);
			*/

			const AnimationRepository* get_repository(Registry& registry, Entity entity) const;
			const AnimationData::AnimationContainer* get_slices_from_asset(Registry& registry, Entity entity) const;
			const AnimationData* get_data_from_asset(Registry& registry, Entity entity) const;
	};
}