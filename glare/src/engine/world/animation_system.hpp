#pragma once

namespace engine
{
	class World;
	class Service;

	struct Animator;

	class AnimationSystem
	{
		public:
			void subscribe(Service& svc);
			void update(World& world, float delta_time);
	};
}