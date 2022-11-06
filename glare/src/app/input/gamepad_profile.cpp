#include "gamepad_profile.hpp"
#include "profile_metadata.hpp"

//#include "gamepad_state.hpp"
#include "gamepad_buttons.hpp"
#include "gamepad_analog.hpp"

#include "input_profile_impl.hpp"

#include <util/magic_enum.hpp>

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

		analog.invert_x = util::get_value(data, "invert_x", analog.invert_x);
		analog.invert_y = util::get_value(data, "invert_y", analog.invert_y);
		//analog.invert_z = util::get_value(data, "invert_z", analog.invert_z);
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
			magic_enum::enum_for_each<GamepadAnalog>([&](auto analog)
			{
				auto* analog_out = deadzone.get_analog(analog);

				if (!analog_out)
				{
					return;
				}

				const auto analog_name = magic_enum::enum_name<GamepadAnalog>(analog);

				util::retrieve_from(*deadzone_data, analog_name, *analog_out, read_analog);
			});
		}

		input_profile_impl::profile_load_basics(*this, profile_metadata, json);
	}
}