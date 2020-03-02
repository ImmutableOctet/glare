#pragma once

//#include <app/input/types.hpp>

#include "types.hpp"
#include "transform.hpp"

#include "world/world.hpp"
//#include "world/entity.hpp"

namespace app
{
	namespace input
	{
		struct MouseState;
	}
}

namespace engine
{
	struct FreeLook
	{
		public:
			using MouseState = app::input::MouseState;

			static void update(World& world, const MouseState& mouse_state);

			//int x = 0;
			//int y = 0;

			// Mouse sensitivity. (In radians)
			float sensitivity = glm::radians(0.1f); // 2 degrees.
		protected:
			void apply(World& world, Entity entity, Transform& transform, const MouseState& input);
	};
}