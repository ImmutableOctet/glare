#pragma once

#include "gamepad_deadzone.hpp"

#include <util/json.hpp>

namespace app::input
{
	struct GamepadProfile
	{
		GamepadProfile() = default;
		GamepadProfile(const util::json& json);
		GamepadProfile(const GamepadProfile&) = default;
		GamepadProfile(GamepadProfile&&) noexcept = default;

		void load(const util::json& json);

		GamepadDeadZone deadzone;
	};
}