#pragma once

#include "types.hpp"
#include "input_profile.hpp"

// Keyboard button/scan codes in the form of an enum.
#include "keyboard_buttons.hpp"

// Placeholder type.
#include "keyboard_motion.hpp"

#include <util/json_fwd.hpp>

#include <string>
#include <optional>
//#include <unordered_map>

namespace app::input
{
	struct ProfileMetadata;

	struct KeyboardProfile : public InputProfile<KeyboardButton, KeyboardMotion>
	{
		KeyboardProfile() = default;
		KeyboardProfile(const KeyboardProfile&) = default;
		KeyboardProfile(KeyboardProfile&&) noexcept = default;

		KeyboardProfile& operator=(KeyboardProfile&&) noexcept = default;
		KeyboardProfile& operator=(const KeyboardProfile&) = default;

		KeyboardProfile(const ProfileMetadata& profile_metadata, const util::json& json);

		void load(const ProfileMetadata& profile_metadata, const util::json& json);
	};
}