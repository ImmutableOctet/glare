#pragma once

#include "types.hpp"
#include "virtual_button.hpp"

#include <magic_enum/magic_enum.hpp>
#include <util/json.hpp>
#include <util/string.hpp>
#include <util/small_vector.hpp>

#include <unordered_map>
#include <string>
#include <string_view>

// TODO: Look for better RegEx alternative.
#include <regex>

namespace app::input
{
	// Abstract base-type for input-device profiles.
	template <typename NativeButtonType, typename NativeAnalogType>
	struct InputProfile
	{
		using ButtonType = NativeButtonType;
		using AnalogType = NativeAnalogType;

		using ButtonMapping        = std::unordered_map<NativeButtonType, EngineButtonsRaw>;
		using AnalogMapping        = std::unordered_map<NativeAnalogType, EngineAnalogsRaw>;

		using VirtualButtonContainer = util::small_vector<VirtualButton, 4>;

		using VirtualButtonMapping = std::unordered_map<NativeAnalogType, VirtualButtonContainer>;

		ButtonMapping button_mapping;
		AnalogMapping analog_mapping;
		VirtualButtonMapping virtual_button_mapping;

		// TODO: Migrate 'Hat' functionality to this type.
		
		/*
			`resolve` must be a callable taking in an `std::string`, and
			returning an `std::optional<InternalMappingType::value_type>`-like type.
			
			`transform_engine_value` must be a callable taking in:
			* a reference to the object being transformed (element of `mappings_out`)
			* the result of `resolve` ('optional-like' object),
			* the `data_in`'s `value_type`,
			* and an `std::string` input to be processed (same input as `resolve`).
		*/
		template <typename InternalMappingType, typename InputDataType, typename ResolveFn, typename TransformFn>
		static void translate_map(InternalMappingType& mappings_out, const InputDataType& data_in, const util::json& json, ResolveFn&& resolve, TransformFn&& transform_engine_value)
		{
			if (data_in.empty())
			{
				return;
			}

			for (const auto& engine_entry : data_in)
			{
				const auto& engine_name  = engine_entry.first;
				const auto& engine_value = engine_entry.second;

				auto it = json.find(engine_name);

				if (it == json.end())
				{
					continue;
				}

				const auto& entry = *it;
					
				if (entry.empty())
				{
					continue;
				}

				auto process_entry = [&resolve, &transform_engine_value, &mappings_out, &engine_value](const auto& single_entry)
				{
					const auto device_element = single_entry.get<std::string>();

					if (device_element.empty())
					{
						return;
					}

					const auto native_value = resolve(device_element);

					if (native_value)
					{
						transform_engine_value(mappings_out[*native_value], native_value, engine_value, device_element);
					}
				};

				switch (entry.type())
				{
					case util::json::value_t::array:
					{
						for (const auto& proxy : entry.items())
						{
							const auto& single_entry = proxy.value();

							process_entry(single_entry);
						}

						break;
					}
					case util::json::value_t::string:
					{
						process_entry(entry);

						break;
					}

					default:
						continue;
				}
			}
		}

		// See main (detailed) overload for usage notes.
		template <typename InternalMappingType, typename InputDataType, typename ResolveFn>
		static void translate_map(InternalMappingType& mappings_out, const InputDataType& data_in, const util::json& json, ResolveFn&& resolve)
		{
			translate_map(mappings_out, data_in, json, resolve, [](auto& output, const auto& native_value, const auto& engine_value, const auto& device_element)
			{
				output = engine_value;
			});
		}

		// See main (detailed) overload for usage notes.
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

		// Reads 'virtual button' mappings (analog -> button construct) from a JSON object.
		// TODO: Look into possible optimizations of `match[INDEX].str()` usage.
		inline void read_virtual_button_mapping(const EngineButtonMap& buttons, const util::json& json)
		{
			const auto virtual_button_rgx = std::regex("([\\w]+)(\\.([xyz\\|]+))?(([\\>|\\<|\\|])?([\\+\\-\\d\\.]+)?)");
			std::smatch rgx_match;

			translate_map
			(
				virtual_button_mapping,
				buttons, json,
				
				// Resolve:
				[&virtual_button_rgx, &rgx_match](const auto& device_element) -> std::optional<NativeAnalogType>
				{
					if (std::regex_search(device_element.begin(), device_element.end(), rgx_match, virtual_button_rgx))
					{
						// TODO: Look into possible optimizations.
						const auto& element_name = rgx_match[1].str();

						return magic_enum::enum_cast<NativeAnalogType>(element_name);
					}

					return std::nullopt;
				},

				// Transform:
				[&rgx_match](auto& output, const auto& native_value, const auto& engine_value, const auto& device_element)
				{
					assert(!rgx_match.empty());

					const auto number_of_matches = rgx_match.size();

					float threshold = 0.5f;

					VirtualButton::AxisRaw axis = 0;

					auto comparison_method = VirtualButton::ComparisonMethod::Both;

					// NOTE: Fixed RegEx indices are based on the expression above. (`virtual_button_rgx`)
					if (number_of_matches > 3)
					{
						const auto axis_str = util::uppercase(rgx_match[3].str());

						for (const auto& axis_symbol : axis_str)
						{
							const auto axis_symbol_value = magic_enum::enum_cast<VirtualButton::Axis>
							(
								// NOTE: Raw character values are interpreted as integral and therefore need to
								// be wrapped in an `std::string_view` for `magic_enum` to read them.
								std::string_view { &axis_symbol, 1 }
							);

							if (!axis_symbol_value)
							{
								continue;
							}

							axis |= static_cast<VirtualButton::AxisRaw>(*axis_symbol_value);
						}

						if (number_of_matches > 5)
						{
							const auto comparison_indicator = rgx_match[5].str();

							switch (comparison_indicator[0])
							{
								case '<':
									comparison_method = VirtualButton::ComparisonMethod::Lesser;

									break;
								case '>':
									comparison_method = VirtualButton::ComparisonMethod::Greater;
									
									break;
								//default:
								//	comparison_method = VirtualButton::ComparisonMethod::Both;
								//	break;
							}

							if (number_of_matches > 6)
							{
								const auto threshold_input = rgx_match[6].str();
								threshold = std::stof(threshold_input);
							}
						}
					}

					// Finalized axis value cannot be zero.
					// (Invalid for `VirtualButton::Axis`)
					assert((axis != 0));

					// Output to the small vector type dedicated to this analog.
					output.emplace_back
					(
						engine_value,
						threshold,
						static_cast<VirtualButton::Axis>(axis),
						comparison_method
					);
				}
			);
		}
	};
}