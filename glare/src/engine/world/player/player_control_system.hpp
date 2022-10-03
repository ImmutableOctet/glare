#pragma once

#include <engine/world/world_system.hpp>

namespace engine
{
	struct OnButtonDown;

	class PlayerControlSystem : public WorldSystem
	{
		public:
			PlayerControlSystem(World& world);

			void on_subscribe(World& world) override;
			void on_update(World& world, float delta) override;

			void on_button_down(const OnButtonDown& data);
		protected:
			// [Insert action queue and history here]
	};
}