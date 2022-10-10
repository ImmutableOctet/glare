#pragma once

#include "types.hpp"
#include "input_profile.hpp"

// Keyboard button/scan codes in the form of an enum.
#include "keyboard_buttons.hpp"

// Keyboard Hat switch simulation.
#include "keyboard_hat.hpp"

// Placeholder type.
#include "keyboard_motion.hpp"

#include <util/json.hpp>

#include <string>
#include <optional>
#include <vector>
//#include <unordered_map>

namespace app::input
{
	struct ProfileMetadata;

	struct KeyboardProfile : public InputProfile<KeyboardButton, KeyboardMotion>
	{
		using Hat = KeyboardHat;
		using HatContainer = std::vector<Hat>;

		KeyboardProfile() = default;
		KeyboardProfile(const KeyboardProfile&) = default;
		KeyboardProfile(KeyboardProfile&&) noexcept = default;

		KeyboardProfile& operator=(KeyboardProfile&&) noexcept = default;
		KeyboardProfile& operator=(const KeyboardProfile&) = default;

		KeyboardProfile(const ProfileMetadata& profile_metadata, const util::json& json);

		void load(const ProfileMetadata& profile_metadata, const util::json& json);
		void load_hats(const ProfileMetadata& profile_metadata, const util::json& json);

		std::optional<std::size_t> get_hat_index(std::string_view hat_name) const;
		const Hat* get_hat(std::string_view hat_name) const;

		const HatContainer& get_hats() const { return hat_descriptors; }
		HatContainer& get_hats() { return hat_descriptors; }

		HatContainer hat_descriptors; // std::unordered_map<std::string, Hat>
	};
}