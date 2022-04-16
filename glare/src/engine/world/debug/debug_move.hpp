#pragma once

#include <math/math.hpp>
#include <engine/types.hpp>

namespace app
{
	namespace input
	{
		struct KeyboardState;
	}
}

namespace engine
{
	struct Transform;

	namespace debug
	{
		struct DebugMove
		{
			public:
				enum SpeedPreset
				{
					Normal,
					Fast,
					Slow
				};

				using KeyboardState = app::input::KeyboardState;

				static void update(World& world, const KeyboardState& keyboard_state);

				math::Vector movement_speed = { 0.8f, 1.0f, 0.8f };

				//SpeedPreset speed_preset = SpeedPreset::Normal;
			protected:
				void apply(World& world, Entity entity, Transform& transform, const KeyboardState& input);
		};
	}
}