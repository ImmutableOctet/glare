#include "keyboard_profile.hpp"
#include "profile_metadata.hpp"

#include "input_profile_impl.hpp"

#include <optional>

namespace app::input
{
	KeyboardProfile::KeyboardProfile(const ProfileMetadata& profile_metadata, const util::json& json)
		: KeyboardProfile()
	{
		load(profile_metadata, json);
	}

	void KeyboardProfile::load(const ProfileMetadata& profile_metadata, const util::json& json)
	{
		load_hats(profile_metadata, json);

		//input_profile_impl::profile_load_basics(*this, profile_metadata, json);

		if (const auto button_data = json.find("buttons"); button_data != json.end())
		{
			read_button_mapping(profile_metadata.buttons, *button_data);
		}

		if (const auto analog_data = json.find("analogs"); analog_data != json.end())
		{
			//read_analog_mapping(profile_metadata.analogs, *analog_data);

			const auto& analogs = profile_metadata.analogs;

			translate_map
			(
				analog_mapping,
				analogs,
				*analog_data,

				[this](const auto& element_name) -> std::optional<AnalogType>
				{
					auto index = get_hat_index(element_name);

					if (index)
					{
						return static_cast<AnalogType>(*index); // KeyboardMotion
					}

					return std::nullopt;
				}
			);
		}
	}

	void KeyboardProfile::load_hats(const ProfileMetadata& profile_metadata, const util::json& json)
	{
		if (const auto hat_data = json.find("hats"); hat_data != json.end())
		{
			const auto& hats = *hat_data;

			for (const auto& proxy : hats.items())
			{
				const auto& hat_entry = proxy.value();

				auto hat_name = hat_entry["name"].get<std::string>();

				if (hat_name.empty())
				{
					continue;
				}

				hat_descriptors.emplace_back(hat_entry, hat_name);
			}
		}
	}

	std::optional<std::size_t> KeyboardProfile::get_hat_index(std::string_view hat_name) const
	{
		for (std::size_t i = 0; i < hat_descriptors.size(); i++)
		{
			const auto& hat = hat_descriptors[i];

			if (hat.name == hat_name)
			{
				return i;
			}
		}

		return std::nullopt;
	}

	const KeyboardProfile::Hat* KeyboardProfile::get_hat(std::string_view hat_name) const
	{
		for (const auto& hat : hat_descriptors)
		{
			if (hat.name == hat_name)
			{
				return &hat;
			}
		}

		return nullptr;
	}
}