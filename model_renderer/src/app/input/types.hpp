#pragma once

#include <array>

#include <types.hpp>

namespace app
{
	namespace input
	{
		template <std::size_t num_buttons>
		struct DeviceState
		{
			static const std::size_t button_count = num_buttons;

			std::array<bool, button_count> buttons;
		};
	}
}