#pragma once

#include <engine/types.hpp>
#include <engine/world/world_system.hpp>

namespace engine
{
	class World;
	class Service;

	struct Animator;
	struct AnimationTransition;

	class AnimationSystem : public WorldSystem
	{
		public:
			AnimationSystem(World& world);

			using Transition = AnimationTransition;

			void on_subscribe(World& world) override;
			void on_update(World& world, float delta_time) override;
		protected:
			// May need this in the future.
			//World& world;
	};
}