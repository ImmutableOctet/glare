#pragma once

//#include <app/input/types.hpp>

#include <math/math.hpp>
#include "types.hpp"

namespace app
{
	namespace input
	{
		struct MouseState;
	}
}

namespace engine
{
	struct Transform;

	struct FreeLook
	{
		public:
			using MouseState = app::input::MouseState;

			static void update(World& world, const MouseState& mouse_state);

			//int x = 0;
			//int y = 0;

			// Mouse sensitivity. (In radians)
			float sensitivity = glm::radians(0.05f);
		protected:
			void apply(World& world, Entity entity, Transform& transform, const MouseState& input);
	};
}