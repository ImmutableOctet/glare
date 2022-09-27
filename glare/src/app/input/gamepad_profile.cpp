#include "gamepad_profile.hpp"

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

	GamepadProfile::GamepadProfile(const util::json& json)
		: GamepadProfile()
	{
		load(json);
	}

	void GamepadProfile::load(const util::json& json)
	{
		if (auto deadzone_data = json.find("deadzone"); deadzone_data != json.end())
		{
			util::retrieve_from(*deadzone_data, "left_analog", deadzone.left_analog, read_analog);
			util::retrieve_from(*deadzone_data, "right_analog", deadzone.right_analog, read_analog);
			util::retrieve_from(*deadzone_data, "triggers", deadzone.triggers, read_analog);
		}
	}
}