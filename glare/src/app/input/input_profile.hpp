#pragma once

#include "types.hpp"
#include "virtual_button.hpp"
#include "hat.hpp"

#include <util/magic_enum.hpp>
#include <util/json.hpp>
#include <util/string.hpp>
#include <util/small_vector.hpp>

#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <optional>

// TODO: Look for better RegEx alternative.
#include <regex>

namespace app::input
{
	// Abstract base-type for input-device profiles.
	template <typename NativeButtonType, typename NativeAnalogType>
	struct InputProfile
	{
		using ButtonType   = NativeButtonType;
		using AnalogType   = NativeAnalogType;

		using Hat          = Hat<NativeButtonType>;
		using HatContainer = std::vector<Hat>; // util::small_vector<Hat, 4>;

		using ButtonMapping        = std::unordered_map<NativeButtonType, EngineButtonsRaw>;
		using AnalogMapping        = std::unordered_map<NativeAnalogType, EngineAnalogsRaw>;

		using VirtualButtonContainer = util::small_vector<VirtualButton, 4>;

		using VirtualButtonMapping = std::unordered_map<NativeAnalogType, VirtualButtonContainer>;

		static constexpr AnalogType hat_index_to_analog_type(std::size_t hat_index)
		{
			return static_cast<AnalogType>
			(
				(hat_index)
				+
				static_cast<std::size_t> // May change this later.
				(AnalogType::RuntimeAnalogOffset)
			);
		}

		ButtonMapping button_mapping;
		AnalogMapping analog_mapping;
		VirtualButtonMapping virtual_button_mapping;

		HatContainer hat_descriptors; // std::unordered_map<std::string, Hat>

		inline const HatContainer& get_hats() const { return hat_descriptors; }
		inline HatContainer& get_hats() { return hat_descriptors; }
		inline bool has_hats() const { return !hat_descriptors.empty(); }

		// NOTE: This does not generate any kind of event.
		inline void clear_hat_activity()
		{
			for (auto& hat : hat_descriptors)
			{
				hat.set_active(false);
			}
		}
		
		inline std::optional<std::size_t> get_hat_index(std::string_view hat_name) const
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

		inline std::optional<AnalogType> get_hat_index_as_analog(std::string_view hat_name) const
		{
			const auto hat_index = get_hat_index(hat_name);

			if (hat_index)
			{
				return hat_index_to_analog_type(*hat_index);
			}

			return std::nullopt;
		}

		// Returns a temporary pointer to the first Hat descriptor found matching `hat_name`.
		// If no match could be found, this returns `nullptr`.
		inline const Hat* get_hat(std::string_view hat_name) const
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
			translate_map(analog_mapping, analogs, json, [this](const auto& element_name) -> std::optional<NativeAnalogType>
			{
				if (const auto analog = magic_enum::enum_cast<NativeAnalogType>(element_name))
				{
					return analog;
				}

				if (const auto hat_analog = get_hat_index_as_analog(element_name))
				{
					return hat_analog;
				}

				return std::nullopt;
			});
		}

		// Reads 'virtual button' mappings (analog -> button construct) from a JSON object.
		// TODO: Look into possible optimizations of `match[INDEX].str()` usage.
		inline void read_virtual_button_mapping(const EngineButtonMap& buttons, const util::json& json, bool check_hats=true) // check_hats=false
		{
			const auto virtual_button_rgx = std::regex("([\\w]+)(\\.([xyz\\|]+))?(([\\>|\\<|\\|])?([\\+\\-\\d\\.]+)?)");
			std::smatch rgx_match;

			translate_map
			(
				virtual_button_mapping,
				buttons, json,
				
				// Resolve:
				[this, &virtual_button_rgx, &rgx_match, &check_hats](const auto& device_element) -> std::optional<NativeAnalogType>
				{
					if (std::regex_search(device_element.begin(), device_element.end(), rgx_match, virtual_button_rgx))
					{
						// TODO: Look into possible optimizations.
						const auto& element_name = rgx_match[1].str();

						if (const auto result = magic_enum::enum_cast<NativeAnalogType>(element_name))
						{
							return result;
						}

						if (check_hats)
						{
							if (const auto hat_analog = get_hat_index_as_analog(element_name))
							{
								return hat_analog;
							}
						}

						return std::nullopt;
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
							// NOTE: Raw character values are interpreted as integral and therefore need to
							// be wrapped in an `std::string_view` for `magic_enum` to read them.
							const auto axis_symbol_view = std::string_view { &axis_symbol, 1 };
							const auto axis_symbol_value = magic_enum::enum_cast<VirtualButton::Axis>(axis_symbol_view);

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
								if (const auto threshold_input = rgx_match[6].str(); !threshold_input.empty())
								{
									threshold = std::stof(threshold_input);
								}
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

		inline void read_hat_descriptors(const util::json& json)
		{
			for (const auto& proxy : json.items())
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
	};
}