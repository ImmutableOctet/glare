#pragma once

/*
	Shared/useful functionality for implementing input profiles.
	Used to share code between `Mouse` and `Keyboard` device types.

	This header is meant to be included in the source/implementation file of an input device.
*/

#include "input_profile.hpp"
#include "profile_metadata.hpp"

#include <string>
#include <string_view>
#include <optional>

#include <util/json.hpp>

namespace app::input
{
	//struct ProfileMetadata;

	namespace input_profile_impl
	{
		// Retrieves the intended profile name for `self_device_name`.
		// If a profile name can't be resolved, this will return `default_profile_name`.
		std::string get_intended_profile_name
		(
			const util::json& data,
			std::string_view device_entries_label,
			std::string_view self_device_name,
			std::string_view default_profile_name="default"
		);

		// Safely looks for `target_profile_name` in `data["profiles"]`,
		// returning a pointer to the profile's JSON data if found.
		const util::json* find_profile_by_name
		(
			const util::json& data,
			const std::string& target_profile_name
		);

		// Loads device-to-player/state-index mappings from JSON data.
		void load_player_mappings
		(
			const ProfileMetadata& profile_metadata,
			const util::json& data,
			std::string_view self_device_name
		);

		// Mono-instance device profile loading.
		template <typename ProfileType>
		const ProfileType* load_profile
		(
			const ProfileMetadata& profile_metadata,        // The profile metadata needed to load this profile.
			std::optional<ProfileType>& profile_out,        // An `std::optional` object derived from `ProfileType`.
			std::string_view device_entries_label,          // e.g. "mice", "keyboards"
			std::string_view self_device_name,              // The (potentially hard-coded) name of this device. -- Used for data validation. // std::string
			std::string_view default_profile_name="default" // The profile we can default to loading from.
		)
		{
			// Load JSON data from disk.
			auto data = util::load_json(profile_metadata.path);

			// Retrieve the intended profile name. (Defaults to "default")
			const auto intended_profile_name = get_intended_profile_name(data, device_entries_label, self_device_name);

			// Look for the intended profile in `data`.
			// NOTE: This usually fails on single-entry configurations.
			const auto* profile_data_ptr = find_profile_by_name(data, intended_profile_name);

			// If we weren't able to find the intended profile via a
			// profile list, check for a single-entry config instead:
			if (!profile_data_ptr)
			{
				if (const auto single_profile_it = data.find("profile"); single_profile_it != data.end())
				{
					// Point to the single-entry provided.
					profile_data_ptr = &(*single_profile_it);
				}
			}

			// If we were successful in identifying a profile entry, read from it:
			if (profile_data_ptr)
			{
				profile_out = ProfileType(profile_metadata, *profile_data_ptr);
			}

			// Load device-to-player mappings:
			load_player_mappings(profile_metadata, data, self_device_name);

			// Return the current device profile.
			// (Likely to have been changed above):
			if (profile_out)
			{
				return &(profile_out.value());
			}

			// If no profile could be resolved, return `nullptr`.
			return nullptr;
		}

		// Basic loading support for `InputProfile` types.
		template <typename ProfileType>
		void profile_load_basics(ProfileType& self, const ProfileMetadata& profile_metadata, const util::json& json)
		{
			if (const auto button_data = json.find("buttons"); button_data != json.end())
			{
				self.read_button_mapping(profile_metadata.buttons, *button_data);
			}

			if (const auto analog_data = json.find("analogs"); analog_data != json.end())
			{
				self.read_analog_mapping(profile_metadata.analogs, *analog_data);
			}
		}
	}
}