#pragma once

#include "types.hpp"

#include <magic_enum/magic_enum.hpp>
#include <util/json.hpp>

#include <unordered_map>

namespace app::input
{
	// Abstract base-type for input-device profiles.
	template <typename NativeButtonType, typename NativeAnalogType>
	struct InputProfile
	{
		public:
			using ButtonMapping = std::unordered_map<NativeButtonType, EngineButtonsRaw>;
			using AnalogMapping = std::unordered_map<NativeAnalogType, EngineAnalogsRaw>;

			ButtonMapping button_mapping;
			AnalogMapping analog_mapping;
		protected:
			template <typename NativeEnumType, typename InternalMappingType, typename InputDataType>
			static void translate_map(InternalMappingType& mappings_out, const InputDataType& data_in, const util::json& json)
			{
				if (data_in.empty())
				{
					return;
				}

				for (const auto& engine_entry : data_in)
				{
					const auto& engine_name = engine_entry.first;
					const auto& engine_value  = engine_entry.second;
					const auto gamepad_element_name = util::get_value<std::string>(json, engine_name);

					if (gamepad_element_name.empty())
					{
						continue;
					}

					const auto native_value = magic_enum::enum_cast<NativeEnumType>(gamepad_element_name);

					if (native_value)
					{
						mappings_out[*native_value] = engine_value;
					}
				}
			}

			// Reads button mappings from a JSON object. (Usually called "buttons")
			// This leverages the `magic_enum` library's string-to-enum conversion functionality.
			inline void read_button_mapping(const EngineButtonMap& buttons, const util::json& json)
			{
				translate_map<NativeButtonType>(button_mapping, buttons, json);
			}

			inline void read_analog_mapping(const EngineAnalogMap& analogs, const util::json& json)
			{
				translate_map<NativeAnalogType>(analog_mapping, analogs, json);
			}
	};
}