#pragma once

#include <engine/types.hpp>
#include <engine/world/world_system.hpp>

namespace engine
{
	class World;
	class Service;

	struct Animator;
	struct AnimationTransition;

	struct OnParentChanged;
	struct BoneComponent;

	class AnimationSystem : public WorldSystem
	{
		public:
			AnimationSystem(World& world);

			using Transition = AnimationTransition;

			void on_subscribe(World& world) override;
			void on_update(World& world, float delta_time) override;
		protected:
			// Used internally by `on_parent_changed`.
			void set_bone_skeleton(Registry& registry, Entity entity, BoneComponent& bone_component, Entity new_skeleton);

			void on_parent_changed(const OnParentChanged& parent_changed);
	};
}