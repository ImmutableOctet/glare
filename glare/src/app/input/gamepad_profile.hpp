#pragma once

#include "types.hpp"
#include "input_profile.hpp"

#include "gamepad_analog.hpp"
#include "gamepad_deadzone.hpp"
#include "gamepad_buttons.hpp"

#include <util/json.hpp>
//#include "button_mapping.hpp"

#include <unordered_map>

namespace app::input
{
	struct ProfileMetadata;

	//using ButtonMappings = std::unordered_map<std::string, std::string>;

	// Mapping between gamepad-native button indices and engine-defined button indices.
	using GamepadButtonMapping = InputProfile<GamepadButtonBits, GamepadAnalog>::ButtonMapping; // std::unordered_map<GamepadButtonBits, EngineButtonsRaw>;

	struct GamepadProfile : public InputProfile<GamepadButtonBits, GamepadAnalog>
	{
		GamepadProfile() = default;
		GamepadProfile(const GamepadProfile&) = default;
		GamepadProfile(GamepadProfile&&) noexcept = default;

		GamepadProfile(const ProfileMetadata& profile_metadata, const util::json& json); // , button_definitions

		GamepadProfile& operator=(GamepadProfile&&) noexcept = default;
		GamepadProfile& operator=(const GamepadProfile&) = default;

		void load(const ProfileMetadata& profile_metadata, const util::json& json);

		GamepadDeadZone deadzone;
	};
}