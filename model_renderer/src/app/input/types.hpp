#pragma once

#include <array>

#include <types.hpp>
#include <engine/types.hpp>

#include "input_device.hpp"

namespace app
{
	namespace input
	{
		struct MouseState
		{
			int x = 0;
			int y = 0;

			bool left = false;
			bool middle = false;
			bool right = false;
		};

		struct KeyboardState
		{
			public:
				using key_value_t = std::uint8_t;

				const key_value_t* keys = nullptr;

				inline bool has_keys() const { return (keys != nullptr); }
		};
	}
}