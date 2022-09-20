#pragma once

#include <array>

#include <types.hpp>

#include "input_device.hpp"

namespace app::input
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
			int num_keys = 0; // std::uint32_t

			bool get_key(int scan_code) const;
			bool has_keys() const;
	};
}