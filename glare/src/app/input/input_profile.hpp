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
		using ButtonType = NativeButtonType;
		using AnalogType = NativeAnalogType;

		using ButtonMapping = std::unordered_map<NativeButtonType, EngineButtonsRaw>;
		using AnalogMapping = std::unordered_map<NativeAnalogType, EngineAnalogsRaw>;

		ButtonMapping button_mapping;
		AnalogMapping analog_mapping;
			
		// `resolve` must be a callable taking in an `std::string` and returning an `std::optional<InternalMappingType::value_type>`-like type.
		template <typename InternalMappingType, typename InputDataType, typename ResolveFn>
		static void translate_map(InternalMappingType& mappings_out, const InputDataType& data_in, const util::json& json, ResolveFn&& resolve)
		{
			if (data_in.empty())
			{
				return;
			}

			for (const auto& engine_entry : data_in)
			{
				const auto& engine_name  = engine_entry.first;
				const auto& engine_value = engine_entry.second;

				const auto element_name = util::get_value<std::string>(json, engine_name);

				if (element_name.empty())
				{
					continue;
				}

				const auto native_value = resolve(element_name);

				if (native_value)
				{
					mappings_out[*native_value] = engine_value;
				}
			}
		}

		template <typename NativeEnumType, typename InternalMappingType, typename InputDataType>
		static void translate_map(InternalMappingType& mappings_out, const InputDataType& data_in, const util::json& json)
		{
			translate_map(mappings_out, data_in, json, [](const auto& element_name)
			{
				return magic_enum::enum_cast<NativeEnumType>(element_name);
			});
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