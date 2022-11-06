#pragma once

#include "types.hpp"
#include "input_profile.hpp"

#include "mouse_buttons.hpp"
#include "mouse_motion.hpp"

#include <util/json.hpp>

namespace app::input
{
	struct ProfileMetadata;

	struct MouseProfile : public InputProfile<MouseButton, MouseMotion>
	{
		using MotionType = AnalogType;

		MouseProfile() = default;
		MouseProfile(const MouseProfile&) = default;
		MouseProfile(MouseProfile&&) noexcept = default;

		MouseProfile& operator=(MouseProfile&&) noexcept = default;
		MouseProfile& operator=(const MouseProfile&) = default;

		MouseProfile(const ProfileMetadata& profile_metadata, const util::json& json);

		void load(const ProfileMetadata& profile_metadata, const util::json& json);

		// Applies post-processing to `input`, returning a new object as a result.
		// (Handles axis inversion, etc.)
		MouseAnalogInput get_analog_value(MouseAnalogInput input) const;

		// Applies post-processing to `x` and `y`, returning a new object as a result.
		inline MouseAnalogInput get_analog_value(MouseAnalogInputRaw x, MouseAnalogInputRaw y) const
		{
			return get_analog_value({x, y});
		}

		// TODO: Look into 'deadzones' for mice.
		// (Minimum required movement)
		//MouseDeadZone deadzone;

		float sensitivity = 2.0f;

		bool invert_x : 1 = false;
		bool invert_y : 1 = false;
	};
}