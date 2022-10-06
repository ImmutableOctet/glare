#include "input_profile_impl.hpp"

namespace app::input
{
	namespace input_profile_impl
	{
		std::string get_intended_profile_name
		(
			const util::json& data,
			std::string_view device_entries_label,
			std::string_view self_device_name,
			std::string_view default_profile_name
		)
		{
			if (const auto devices_it = data.find(device_entries_label); devices_it != data.end())
			{
				const auto& device_entries = *devices_it;

				for (const auto& proxy : device_entries.items())
				{
					const auto& device_entry = proxy.value();

					const auto device_name = device_entry["device_name"].get<std::string>();

					if (device_name == self_device_name)
					{
						return device_entry["profile"].get<std::string>();
					}
				}
			}

			return std::string { default_profile_name };
		}

		const util::json* find_profile_by_name
		(
			const util::json& data,
			const std::string& target_profile_name // std::string_view
		)
		{
			if (const auto profile_entries_it = data.find("profiles"); profile_entries_it != data.end())
			{
				const auto& profile_entries = *profile_entries_it;

				for (const auto& proxy : profile_entries.items())
				{
					const auto& profile_entry = proxy.value();

					const auto  profile_name = profile_entry["name"].get<std::string>();

					if (profile_name == target_profile_name)
					{
						return &profile_entry;
					}
				}
			}

			return nullptr;
		}

		void load_player_mappings
		(
			const ProfileMetadata& profile_metadata,
			const util::json& data,
			std::string_view self_device_name
		)
		{
			if (const auto players_it = data.find("players"); players_it != data.end())
			{
				const auto& players = *players_it;

				for (const auto& proxy : players.items())
				{
					const auto& player_entry = proxy.value();

					auto device_name = player_entry["device_name"].get<std::string>();
					auto player_index = player_entry["player_index"].get<PlayerInputID>();

					// If the device specified is different from this device, disregard this entry:
					if (device_name != self_device_name)
					{
						// NOTE: `peek_device_name` currently returns a pre-defined value.
						// This means that we will only accept entries that use this value.
						continue;
					}

					// Apply device-to-player mapping.
					profile_metadata.player_mappings_out[device_name] = player_index;
				}
			}
		}
	}
}