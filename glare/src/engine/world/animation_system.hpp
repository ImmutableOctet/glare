#pragma once

#include <engine/types.hpp>

namespace engine
{
	class World;
	class Service;

	struct Animator;
	struct AnimationTransition;

	class AnimationSystem
	{
		public:
			using Transition = AnimationTransition;

			void subscribe(Service& svc);
			void update(World& world, float delta_time);
	};
}