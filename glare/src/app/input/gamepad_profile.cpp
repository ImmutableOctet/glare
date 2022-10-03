#include "gamepad_profile.hpp"
#include "profile_metadata.hpp"

//#include "gamepad_state.hpp"
#include "gamepad_buttons.hpp"

#include <cmath>

namespace app::input
{
	static void read_range(const util::json& data, std::optional<GamepadDeadZone::Range>& range)
	{
		auto min       = math::negative(util::get_value(data, "min", GamepadDeadZone::MIN_AXIS_VALUE_NORMALIZED));
		auto max       = math::positive(util::get_value(data, "max", GamepadDeadZone::MAX_AXIS_VALUE_NORMALIZED));
		auto threshold = std::abs(util::get_value(data, "threshold", 0.0f));

		range = { threshold, (min+threshold), (max-threshold) };
	}

	static void read_analog(const util::json& data, GamepadDeadZone::Analog& analog)
	{
		util::retrieve_from(data, "x", analog.x, read_range);
		util::retrieve_from(data, "y", analog.y, read_range);
	}

	GamepadProfile::GamepadProfile(const ProfileMetadata& profile_metadata, const util::json& json)
		: GamepadProfile()
	{
		load(profile_metadata, json);
	}

	void GamepadProfile::load(const ProfileMetadata& profile_metadata, const util::json& json)
	{
		if (const auto deadzone_data = json.find("deadzones"); deadzone_data != json.end())
		{
			util::retrieve_from(*deadzone_data, "Left", deadzone.left_analog, read_analog); // "left_analog"
			util::retrieve_from(*deadzone_data, "Right", deadzone.right_analog, read_analog); // "right_analog"
			util::retrieve_from(*deadzone_data, "Triggers", deadzone.triggers, read_analog); // "triggers"
		}

		if (const auto button_data = json.find("buttons"); button_data != json.end())
		{
			read_button_mapping(profile_metadata.buttons, *button_data);
		}

		if (const auto analog_data = json.find("analogs"); analog_data != json.end())
		{
			read_analog_mapping(profile_metadata.analogs, *analog_data);
		}
	}
}