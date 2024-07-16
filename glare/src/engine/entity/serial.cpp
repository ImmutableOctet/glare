#include "serial.hpp"
#include "serial_impl.hpp"

#include "entity_descriptor.hpp"
#include "entity_descriptor_shared.hpp"
#include "entity_target.hpp"
#include "entity_state.hpp"
#include "entity_state_rule.hpp"
#include "entity_state_collection.hpp"
#include "entity_thread_description.hpp"
#include "entity_thread_builder.hpp"
#include "event_trigger_condition.hpp"
#include "meta_description.hpp"

//#include <engine/meta/meta.hpp>
#include <engine/meta/types.hpp>
#include <engine/meta/serial.hpp>
#include <engine/meta/hash.hpp>
#include <engine/meta/data_member.hpp>
#include <engine/meta/short_name.hpp>

#include <engine/meta/meta_type_descriptor.hpp>
#include <engine/meta/meta_type_descriptor_flags.hpp>
#include <engine/meta/meta_type_resolution_context.hpp>
#include <engine/meta/meta_variable.hpp>
#include <engine/meta/meta_variable_context.hpp>
#include <engine/meta/meta_data_member.hpp>
#include <engine/meta/meta_parsing_context.hpp>
#include <engine/meta/indirect_meta_data_member.hpp>

#include <engine/world/animation/animation_slice.hpp>

#include <util/algorithm.hpp>
#include <util/string.hpp>
#include <util/parse.hpp>
#include <util/variant.hpp>
#include <util/io.hpp>

#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <type_traits>
#include <tuple>
#include <array>
#include <memory>
#include <utility>
#include <charconv>

// TODO: Change to better regular expression library.
#include <regex>

// Debugging related:
#include <util/format.hpp>
#include <util/log.hpp>

namespace engine
{
	static std::string get_embedded_name(const util::json& data)
	{
		if (auto name_it = data.find("name"); name_it != data.end())
		{
			return name_it->get<std::string>();
		}

		return {};
	}

	static std::string default_state_name_from_path(const std::filesystem::path& state_path)
	{
		auto filename = state_path.filename(); filename.replace_extension();

		// An embedded name couldn't be found; use the state's filename instead.
		return filename.string();
	}

	static std::optional<EntityStateIndex> get_default_state_index(const EntityDescriptor& descriptor, const util::json& data)
	{
		if (auto default_state = util::find_any(data, "state", "default_state"); default_state != data.end())
		{
			switch (default_state->type())
			{
				case util::json::value_t::string:
				{
					auto state_name = default_state->get<std::string>();
					auto state_name_hash = hash(state_name).value();

					if (auto index = descriptor.get_state_index(state_name_hash))
					{
						return *index;
					}

					break;
				}

				case util::json::value_t::number_integer:
				case util::json::value_t::number_unsigned:
				{
					auto index = default_state->get<EntityStateIndex>();

					if (index < descriptor.states.size())
					{
						return index;
					}

					break;
				}
			}
		}

		return std::nullopt;
	}

	// TODO: Deprecate/remove. (No longer needed for `process_trigger_expression`)
	std::tuple<std::string_view, std::size_t>
	parse_event_type(const std::string& event_type, std::size_t offset) // std::string_view
	{
		const auto event_type_rgx = std::regex("([^\\s\\:\\.\\-\\>\\&\\|\\(\\)]+)\\s*([^\\s\\&\\|\\(\\)]+)?");

		if (std::smatch rgx_match; std::regex_search((event_type.begin() + static_cast<std::ptrdiff_t>(offset)), event_type.end(), rgx_match, event_type_rgx))
		{
			auto updated_offset = static_cast<std::size_t>(rgx_match.suffix().first - event_type.begin());

			auto parsed_view = std::string_view
			{
				(event_type.data() + static_cast<std::ptrdiff_t>(offset)),
				static_cast<std::size_t>(updated_offset)
			};

			auto type_name = util::match_view(parsed_view, rgx_match, 1);

			return { type_name, updated_offset };
		}

		return {};
	}

	std::tuple<std::string_view, bool, std::optional<std::uint8_t>> // EntityFactory::SmallSize
	parse_component_declaration(const std::string& component_declaration, bool default_allow_entry_update) // std::string_view
	{
		using SmallSize = std::uint8_t;
		
		const auto component_rgx = std::regex("(\\+)?([^\\|]+)(\\|(\\d+))?");

		std::string_view name_view = {};
		bool inplace_changes = default_allow_entry_update;
		std::optional<SmallSize> constructor_arg_count = std::nullopt;

		if (std::smatch rgx_match; std::regex_search(component_declaration.begin(), component_declaration.end(), rgx_match, component_rgx))
		{
			if (rgx_match[1].matched)
			{
				inplace_changes = true;
			}

			name_view = util::match_view(component_declaration, rgx_match, 2);

			if (rgx_match[4].matched)
			{
				SmallSize arg_count = 0;

				auto begin = (component_declaration.data() + rgx_match.position(4));
				auto end = (begin + rgx_match.length(4));

				auto result = std::from_chars(begin, end, arg_count);

				if (result.ec == static_cast<std::errc>(0)) // != std::errc::invalid_argument
				{
					constructor_arg_count = arg_count;
				}
			}
		}

		return { name_view, inplace_changes, constructor_arg_count };
	}

	std::optional<EntityTarget> resolve_manual_target(const util::json& update_entry)
	{
		if (auto manual_target_entry = util::find_any(update_entry, "target"); manual_target_entry != update_entry.end())
		{
			const auto manual_target_raw = manual_target_entry->get<std::string>();

			if (auto result = EntityTarget::parse(manual_target_raw))
			{
				return result;
			}
			else
			{
				print_warn("Failed to resolve target: \"{}\"", manual_target_raw);
			}
		}

		return std::nullopt;
	}

	std::string resolve_state_name(const util::json& state_data, const std::filesystem::path& state_path)
	{
		// Check for a top-level `name` field in the imported state:
		if (auto name = get_embedded_name(state_data); !name.empty())
		{
			// Use the embedded name.
			return name;
		}

		return default_state_name_from_path(state_path);
	}

	std::tuple<std::filesystem::path, std::filesystem::path, util::json>
	load_state_data(std::string_view state_path_raw, const std::filesystem::path& base_path, const EntityFactoryContext* opt_context)
	{
		auto state_path = std::filesystem::path(state_path_raw);

		bool should_load_from_state_path = true;

		if (opt_context)
		{
			auto state_path_as_standard_reference = opt_context->resolve_reference(state_path, base_path);

			if (state_path_as_standard_reference.empty())
			{
				should_load_from_state_path = false;

				if (auto state_path_as_script_reference = opt_context->resolve_entity_script_reference(state_path, base_path); !state_path_as_script_reference.empty())
				{
					state_path = std::move(state_path_as_script_reference);
				}
				else if (auto state_path_as_cpp_script_reference = opt_context->resolve_cpp_script_reference(state_path, base_path); !state_path_as_cpp_script_reference.empty())
				{
					state_path = std::move(state_path_as_cpp_script_reference);
				}
			}
			else
			{
				state_path = std::move(state_path_as_standard_reference);
			}
		}

		auto state_data = util::json {};
		
		if (should_load_from_state_path)
		{
			if (!state_path.empty())
			{
				// TODO: Optimize.
				state_data = util::load_json(state_path);
			}
		}

		auto state_base_path = state_path.parent_path();

		return { std::move(state_base_path), std::move(state_path), std::move(state_data) };
	}

	std::size_t process_update_action
	(
		EntityDescriptor& descriptor,
		EntityStateUpdateAction& action_out,
		const util::json& update_entry,
		const MetaParsingContext& opt_parsing_context
	)
	{
		const auto result = process_component_list
		(
			descriptor,
			action_out.updated_components,
			update_entry, {}, // { .force_field_assignment = true },
			opt_parsing_context,
			true, true, true,
			false
		);

		if (auto manual_target = resolve_manual_target(update_entry))
		{
			action_out.target_entity = *manual_target;
		}

		return result;
	}

	std::size_t process_animation_list
	(
		EntityDescriptor& descriptor,
		AnimationRepository& animations_out,
		const util::json& animation_content,

		const MetaParsingContext& opt_parsing_context
	)
	{
		if (!animation_content.is_object())
		{
			return {};
		}

		auto animations_processed = std::size_t {};

		auto furthest_frame_index = animations_out.get_furthest_frame_sliced();

		auto& animation_slices = animations_out.slices;

		for (const auto& animation_proxy : animation_content.items())
		{
			const auto& animation_name = animation_proxy.key();
			const auto& animation_description = animation_proxy.value();

			auto animation_begin = (furthest_frame_index + static_cast<FrameIndex>(1));
			auto animation_end   = animation_begin; // FrameIndex {};

			switch (animation_description.type())
			{
				case util::json::value_t::object:
				{
					if (const auto manual_frame_begin = util::find_any(animation_description, "from", "begin", "start", "first", "left"); manual_frame_begin != animation_description.end())
					{
						if (manual_frame_begin->is_number_integer() || manual_frame_begin->is_number_unsigned())
						{
							animation_begin = manual_frame_begin->get<FrameIndex>();
						}

						if (const auto manual_frame_end = util::find_any(animation_description, "to", "end", "stop", "last", "second", "right"); manual_frame_end != animation_description.end())
						{
							if (manual_frame_end->is_number_integer() || manual_frame_end->is_number_unsigned())
							{
								animation_end = manual_frame_end->get<FrameIndex>();
							}
						}
					}

					if (animation_end <= animation_begin)
					{
						if (const auto length_specification = util::find_any(animation_description, "length", "size", "frames"); length_specification != animation_description.end())
						{
							if (length_specification->is_number_integer() || length_specification->is_number_unsigned())
							{
								animation_end = (animation_begin + length_specification->get<FrameIndex>());
							}
						}
					}

					break;
				}

				case util::json::value_t::number_integer:
				case util::json::value_t::number_unsigned:
				{
					const auto animation_length = animation_description.get<FrameIndex>();

					animation_begin  = furthest_frame_index;
					animation_end    = (animation_begin + animation_length);

					break;
				}
			}

			if (animation_end > animation_begin)
			{
				const auto animation_id = hash(animation_name).value();

				animation_slices[animation_id] = AnimationSlice { animation_begin, animation_end };

				furthest_frame_index = std::max(furthest_frame_index, animation_end);

				animations_processed++;
			}
		}

		return animations_processed;
	}

	std::size_t process_animation_sequence_list
	(
		EntityDescriptor& descriptor,
		AnimationRepository& animations_out,
		const util::json& sequence_list_content,

		const MetaParsingContext& opt_parsing_context
	)
	{
		auto sequences_processed = std::size_t {};

		for (const auto& sequence_proxy : sequence_list_content.items())
		{
			const auto& sequence_name = sequence_proxy.key();
			const auto& sequence_values = sequence_proxy.value();

			const auto sequence_id = hash(sequence_name).value();

			auto& sequences_out = animations_out.sequences[sequence_id];

			// A temporary non-owning pointer to the animation that is actively being processed. (If any)
			AnimationSequenceEntry* active_animation = {};

			util::json_for_each
			(
				sequence_values,
				
				[&sequences_out, &active_animation](const util::json& sequence_entry)
				{
					switch (sequence_entry.type())
					{
						case util::json::value_t::object:
						{
							if (const auto animation_as_object = util::find_any(sequence_entry, "animation", "animation_name", "name"); animation_as_object != sequence_entry.end())
							{
								const auto animation_name = animation_as_object->get<std::string>();
								const auto animation_id = hash(animation_name).value();

								active_animation = &(sequences_out.animations.emplace_back(animation_id));

								if (active_animation)
								{
									if (const auto rate_specified = util::find_any(sequence_entry, "rate", "speed"); rate_specified != sequence_entry.end())
									{
										active_animation->rate = rate_specified->get<float>();
									}

									if (const auto embedded_transition = util::find_any(sequence_entry, "transition", "animation_transition"); embedded_transition != sequence_entry.end())
									{
										engine::load(active_animation->transition, *embedded_transition);

										active_animation = {};
									}
								}
							}
							else if (active_animation)
							{
								engine::load(active_animation->transition, sequence_entry);

								active_animation = {};
							}

							break;
						}
						case util::json::value_t::string:
						{
							const auto animation_name = sequence_entry.get<std::string>();
							const auto animation_id = hash(animation_name).value();

							active_animation = &(sequences_out.animations.emplace_back(animation_id));

							break;
						}
					}
				}
			);

			sequences_processed++;
		}

		return sequences_processed;
	}

	std::size_t process_animation_layer_list
	(
		EntityDescriptor& descriptor,
		AnimationRepository& animations_out,
		const util::json& layer_list_content,

		const MetaParsingContext& opt_parsing_context
	)
	{
		auto layers_processed = std::size_t {};

		util::json_for_each<util::json::value_t::string>
		(
			layer_list_content,

			[&animations_out, &layers_processed](const util::json& layer_entry)
			{
				const auto layer_name = layer_entry.get<std::string>();
				const auto layer_id = hash(layer_name).value();

				if (animations_out.add_layer(layer_id))
				{
					layers_processed++;
				}
			}
		);

		return layers_processed;
	}

	std::size_t process_bone_animation_layer_mapping
	(
		EntityDescriptor& descriptor,
		AnimationRepository& animations_out,
		const util::json& bone_mapping_content,

		const MetaParsingContext& opt_parsing_context
	)
	{
		auto bones_processed = std::size_t {};

		util::json_for_each<util::json::value_t::object, util::json::value_t::array>
		(
			bone_mapping_content,
				
			[&animations_out, &bones_processed](const util::json& bone_entry)
			{
				switch (bone_entry.type())
				{
					case util::json::value_t::object:
					{
						if (const auto layer_name_it = util::find_any(bone_entry, "layer", "animation_layer", "bone_layer", "value"); layer_name_it != bone_entry.end())
						{
							if (const auto bone_name_it = util::find_any(bone_entry, "bone", "bone_name", "id"); bone_name_it != bone_entry.end())
							{
								const auto layer_name = layer_name_it->get<std::string>();
								const auto layer_id = hash(layer_name).value();

								if (const auto layer_mask = animations_out.get_layer_mask(layer_id))
								{
									const auto bone_name = bone_name_it->get<std::string>();
									const auto bone_id = hash(bone_name).value();

									animations_out.bone_layers[bone_id] = *layer_mask;

									bones_processed++;
								}
							}
						}

						break;
					}
					case util::json::value_t::array:
					{
						if (bone_entry.size() >= 2)
						{
							const auto layer_name = bone_entry[1].get<std::string>();
							const auto layer_id = hash(layer_name).value();

							if (const auto layer_mask = animations_out.get_layer_mask(layer_id))
							{
								const auto bone_name = bone_entry[0].get<std::string>();
								const auto bone_id = hash(bone_name).value();

								animations_out.bone_layers[bone_id] = *layer_mask;

								bones_processed++;
							}
						}

						break;
					}
				}
			}
		);

		return bones_processed;
	}

	std::size_t process_component_list
	(
		EntityDescriptor& descriptor,
		MetaDescription& components_out, // EntityDescriptor::TypeInfo&
		const util::json& components,

		const MetaTypeDescriptorFlags& shared_component_flags,
		const MetaParsingContext& opt_parsing_context,

		bool allow_new_entry,
		bool allow_default_entries,
		bool forward_entry_update_condition_to_flags,

		bool ignore_special_symbols,

		std::optional<bool> allow_entry_updates
	)
	{
		// Shorthand for `process_component`.
		auto as_component =
		[
			&descriptor, &components_out, &shared_component_flags, &opt_parsing_context,
			allow_new_entry, allow_default_entries, forward_entry_update_condition_to_flags,
			ignore_special_symbols, allow_entry_updates
		]
		(const auto& component_declaration, const util::json* component_content=nullptr) -> bool
		{
			return process_component
			(
				descriptor,
				components_out,

				component_declaration,
				component_content,
				
				shared_component_flags,
				opt_parsing_context,
				
				allow_new_entry,
				allow_default_entries,
				forward_entry_update_condition_to_flags,

				ignore_special_symbols,

				allow_entry_updates
			);
		};

		std::size_t count = 0;

		util::json_for_each
		(
			components,

			[&as_component, &count](const util::json& comp)
			{
				switch (comp.type())
				{
					case util::json::value_t::object:
					{
						for (const auto& proxy : comp.items())
						{
							const auto& component_declaration = proxy.key();
							const auto& component_content = proxy.value();

							if (as_component(component_declaration, &component_content))
							{
								count++;
							}
						}

						break;
					}
					case util::json::value_t::string:
					{
						const auto component_declaration = comp.get<std::string>();

						if (as_component(component_declaration))
						{
							count++;
						}

						break;
					}
				}
			}
		);

		return count;
	}

	bool process_component
	(
		EntityDescriptor& descriptor,
		MetaDescription& components_out, // EntityDescriptor::TypeInfo&

		// TODO: Change to `std::string_view` (`std::regex` limitation)
		const std::string& component_declaration, // std::string_view

		const util::json* component_content,

		const MetaTypeDescriptorFlags& component_flags,

		const MetaParsingContext& opt_parsing_context,

		bool allow_new_entry,
		bool allow_default_entries,
		bool forward_entry_update_condition_to_flags,

		bool ignore_special_symbols,

		std::optional<bool> allow_entry_update
	)
	{
		auto [component_name, decl_allow_entry_update, constructor_arg_count] = parse_component_declaration(component_declaration);

		if (!ignore_special_symbols)
		{
			// Used by states for `update` actions.
			if (component_name == "target")
			{
				return false;
			}
		}

		const auto opt_type_context = opt_parsing_context.get_type_context();

		auto component_type = (opt_type_context)
			? opt_type_context->get_component_type(component_name)
			: meta_type_from_name(component_name)
		;

		if (!component_type)
		{
			//print("Failed to resolve reflection for symbol: \"{}\"", component_name);

			return false;
		}

		return process_component
		(
			descriptor,
			components_out, // EntityDescriptor::TypeInfo&

			component_type,
			allow_entry_update.value_or(decl_allow_entry_update),
			constructor_arg_count,

			component_content,

			component_flags,

			opt_parsing_context,

			allow_new_entry,
			allow_default_entries,
			forward_entry_update_condition_to_flags
		);
	}

	bool process_component
	(
		EntityDescriptor& descriptor,
		MetaDescription& components_out, // EntityDescriptor::TypeInfo&

		const MetaType& component_type,
		bool allow_entry_update,
		std::optional<std::uint8_t> constructor_arg_count, // EntityFactory::SmallSize

		const util::json* component_content,

		const MetaTypeDescriptorFlags& component_flags,

		const MetaParsingContext& opt_parsing_context,

		bool allow_new_entry,
		bool allow_default_entries,
		bool forward_entry_update_condition_to_flags
	)
	{
		if (!component_type)
		{
			return false;
		}

		const bool force_entry_update = ((!allow_new_entry) && (!allow_default_entries));

		auto make_component = [&]()
		{
			auto flags = component_flags;

			if (forward_entry_update_condition_to_flags && allow_entry_update)
			{
				flags.force_field_assignment = true;
			}

			return process_component
			(
				component_type, component_content,
				constructor_arg_count,
				flags,
				opt_parsing_context,
				&(descriptor.get_shared_storage())
			);
		};

		auto& loaded_components = components_out.type_definitions;

		auto find_existing = [&descriptor, &component_type, &loaded_components]()
		{
			return std::find_if
			(
				loaded_components.begin(), loaded_components.end(),

				[&descriptor, &component_type](const auto& existing_entry) -> bool
				{
					auto& existing_component = existing_entry.get(descriptor);

					if (existing_component.get_type_id() == component_type.id())
					{
						return true;
					}

					return false;
				}
			);
		};

		// Alternative implementation (changes behavior on empty JSON object from default-construction to a no-op):
		//const bool has_component_content = ((component_content) && (!component_content->empty()));

		const bool has_component_content = static_cast<bool>(component_content);

		// Check for an existing instance of this type ID:
		auto it = find_existing();

		if (it == loaded_components.end())
		{
			if ((!allow_new_entry) || ((!allow_default_entries) && (!has_component_content)))
			{
				return false;
			}

			// There's no existing instance, but we're allowed to create a new one.
			loaded_components.emplace_back(descriptor.allocate(make_component()));
		}
		else if (allow_entry_update || force_entry_update)
		{
			auto& existing = it->get(descriptor);

			existing.set_variables(make_component());
		}
		else
		{
			if ((!allow_new_entry) || (!has_component_content) || ((!allow_default_entries) && (!component_content->empty())))
			{
				return false;
			}

			auto new_component_store = make_component();

			auto& existing = it->get(descriptor);

			existing = std::move(new_component_store);
		}

		return true;
	}

	std::optional<MetaTypeDescriptor> process_component
	(
		std::string_view component_name,
		const util::json* data,

		std::optional<std::uint8_t> constructor_arg_count, // SmallSize

		const MetaTypeDescriptorFlags& component_flags,
		const MetaParsingContext& opt_parsing_context,
		SharedStorageInterface* opt_storage
	)
	{
		const auto opt_type_context = opt_parsing_context.get_type_context();

		auto component_type = (opt_type_context)
			? opt_type_context->get_component_type(component_name)
			: meta_type_from_name(component_name)
		;

		if (!component_type)
		{
			//print("Failed to resolve reflection for symbol: \"{}\"", component_name);

			return std::nullopt;
		}

		return process_component
		(
			component_type,
			data,
			constructor_arg_count,
			component_flags,
			opt_parsing_context,
			opt_storage
		);
	}

	MetaTypeDescriptor process_component
	(
		MetaType component_type,
		const util::json* data,
		
		std::optional<std::uint8_t> constructor_arg_count, // SmallSize
		const MetaTypeDescriptorFlags& component_flags,
		const MetaParsingContext& opt_parsing_context,
		SharedStorageInterface* opt_storage
	)
	{
		assert(component_type);

		if (data)
		{
			if (const auto& component_content = *data; !component_content.empty())
			{
				return MetaTypeDescriptor
				(
					component_type, component_content,
					{
						.context = opt_parsing_context,
						.storage = opt_storage,

						// TODO: Look into direct component assignment.
						.fallback_to_component_reference = false,

						.allow_member_references  = true,
						.allow_entity_indirection = true
					},

					constructor_arg_count, component_flags
				);
			}
		}

		return MetaTypeDescriptor
		(
			component_type,
			constructor_arg_count,
			component_flags
		);
	}

	std::size_t process_archetype_component_list
	(
		EntityDescriptor& descriptor,
		const util::json& components,
		const MetaParsingContext& opt_parsing_context,
		bool change_only_if_missing
	)
	{
		if (change_only_if_missing)
		{
			return process_component_list
			(
				descriptor, descriptor.components, components,
				{}, opt_parsing_context,

				true, true, false, true, true
			);
		}
		else
		{
			return process_component_list
			(
				descriptor, descriptor.components, components,
				{}, opt_parsing_context
			);
		}
	}

	MetaAny process_trigger_condition_value
	(
		EntityDescriptor& descriptor,
		std::string_view compared_value_raw,
		const MetaParsingContext& opt_parsing_context
	)
	{
		MetaAny compared_value;

		bool resolve_native_value = true;

		if (util::is_quoted(compared_value_raw, '\"', '\''))
		{
			resolve_native_value = false;

			compared_value_raw = util::unquote(compared_value_raw);
		}
			
		if (compared_value_raw.empty())
		{
			return {};
		}

		if (resolve_native_value)
		{
			compared_value = meta_any_from_string
			(
				compared_value_raw,
				
				{
					.context = opt_parsing_context,
					.storage = &(descriptor.get_shared_storage()),

					.resolve_symbol           = true,
					.strip_quotes             = false,
					.fallback_to_string       = false,
					
					// TODO: Look into direct component comparison.
					.fallback_to_component_reference  = false,

					.allow_member_references          = true, // false
					.allow_entity_indirection         = true,
					.allow_remote_variable_references = true,
					.resolve_command_aliases          = true
				}
			);

			if (!compared_value)
			{
				// TODO: Add support for multiple levels of indirection. (e.g. "child(name)::RelationshipComponent::child(name)")
				// Look for (optionally entity-qualified) reference to data member of component:
				if (auto data_member = indirect_meta_data_member_from_string(compared_value_raw))
				{
					compared_value = allocate_meta_any(*data_member, &(descriptor.get_shared_storage()));
				}
				else
				{
					// Look for reference to entity target:
					if (auto parse_result = EntityTarget::parse_type(compared_value_raw))
					{
						compared_value = allocate_meta_any(EntityTarget { std::move(std::get<0>(*parse_result)) }, &(descriptor.get_shared_storage()));
					}
				}
			}
		}
				
		if (!compared_value)
		{
			//compared_value = std::string(compared_value_raw);
			compared_value = hash(compared_value_raw).value();
		}

		return compared_value;
	}

	std::optional<EventTriggerSingleCondition> process_standard_trigger_condition // std::optional<EventTriggerCondition>
	(
		EntityDescriptor& descriptor,

		const entt::meta_type& type,

		std::string_view member_name,
		std::string_view comparison_operator,
		std::string_view compared_value_raw,

		std::string_view trigger_condition_expr,

		bool embed_type_in_condition,
		bool invert_comparison_operator,

		const MetaParsingContext& opt_parsing_context
	)
	{
		auto [member_id, member] = resolve_trigger_condition_member(type, member_name);

		if (!member && (!member_name.empty()))
		{
			print_warn("Unable to resolve target member for trigger-condition: \"{}\"", trigger_condition_expr);

			return std::nullopt;
		}

		if (!member)
		{
			return std::nullopt;
		}
		
		MetaAny compared_value = {};

		auto comparison_method = (invert_comparison_operator)
			? EventTriggerComparisonMethod::NotEqual
			: EventTriggerComparisonMethod::Equal;

		if (compared_value_raw.empty())
		{
			// If we don't need to embed type information into the trigger-condition, return here.
			if (!embed_type_in_condition)
			{
				// Because type information isn't needed, the caller can interpret
				// the `std::nullopt` return value as an always-true condition.
				// (Usually via `EventTriggerTrueCondition`)
				// 
				// If type information must be preserved, then we have no choice but to create
				// a 'dummy-condition' object to store the type information.
				// (i.e. We fallthrough here, using am empty `MetaAny` object to compare against)
				return std::nullopt;
			}

			// See above notes for details.
			// (Intentionally comparing not-equal against an empty `MetaAny` value)
			comparison_method = (invert_comparison_operator)
				? EventTriggerComparisonMethod::Equal
				: EventTriggerComparisonMethod::NotEqual;
		}
		else
		{
			compared_value = process_trigger_condition_value(descriptor, compared_value_raw, opt_parsing_context);

			if (!compared_value)
			{
				return std::nullopt;
			}

			comparison_method = EventTriggerConditionType::get_comparison_method(comparison_operator, invert_comparison_operator);
		}

		return EventTriggerSingleCondition
		{
			member_id,
			std::move(compared_value),
			comparison_method,

			((embed_type_in_condition) ? type : MetaType {})
		};
	}

	std::optional<EventTriggerMemberCondition> process_member_trigger_condition
	(
		EntityDescriptor& descriptor,

		const entt::meta_type& type,

		std::string_view entity_ref,

		std::string_view member_name,
		std::string_view comparison_operator,
		std::string_view compared_value_raw,

		std::string_view trigger_condition_expr,

		bool invert_comparison_operator,

		const MetaParsingContext& opt_parsing_context
	)
	{
		auto [member_id, member] = resolve_trigger_condition_member(type, member_name);

		if (!member && (!member_name.empty()))
		{
			print_warn("Unable to resolve target member for trigger-condition: \"{}\"", trigger_condition_expr);

			return std::nullopt;
		}

		auto comparison_value = process_trigger_condition_value(descriptor, compared_value_raw, opt_parsing_context);

		if (!comparison_value)
		{
			return std::nullopt;
		}

		auto target = EntityTarget::parse(entity_ref);

		if (!target)
		{
			//target = { EntityTarget::SelfTarget {} };

			return std::nullopt;
		}

		auto comparison_method = EventTriggerConditionType::get_comparison_method(comparison_operator, invert_comparison_operator);

		return EventTriggerMemberCondition
		{
			IndirectMetaDataMember
			{
				std::move(*target),

				MetaDataMember
				{
					type.id(),
					member_id,
				}
			},

			std::move(comparison_value),

			comparison_method
		};
	}

	std::tuple<std::string_view, std::optional<EntityThreadInstruction>, bool>
	parse_instruction_header(std::string_view instruction_raw, const EntityDescriptor* opt_descriptor)
	{
		if (instruction_raw.empty())
		{
			return { instruction_raw, std::nullopt, false };
		}

		// Look for an initial level of indirection:
		auto [initial_ref, remainder, initial_ref_length_parsed] = util::parse_member_reference(instruction_raw, true, true, true, false, true, true);

		if (initial_ref.empty())
		{
			// There's no header, return the raw instruction data and nothing else.
			return { instruction_raw, std::nullopt, false };
		}

		std::string_view thread_ref;
		std::string_view entity_ref;

		std::string_view instruction;

		// Check for a second level of indirection.
		if (auto [second_ref, sub_remainder, second_ref_length_parsed] = util::parse_member_reference(remainder, true, false, true, false, true, true); !second_ref.empty())
		{
			// Use the initial reference as an entity-target.
			entity_ref  = initial_ref;

			// Use the second reference as the thread owned by that entity.
			thread_ref  = second_ref;

			// Use the remaining content for the actual instruction.
			instruction = sub_remainder;

			assert(!instruction.empty());
		}
		else
		{
			// There was only one (initial) reference,
			// assume that this is a thread local to this entity.
			thread_ref  = initial_ref;

			// Use the remaining content for the actual instruction.
			instruction = remainder;
		}

		auto result = parse_thread_details(thread_ref, entity_ref, opt_descriptor);

		// In the event `parse_thread_details` fails, it could mean that
		// we mistook one reference type for the other.
		if (!std::get<0>(result)) // && !thread_ref.empty() && entity_ref.empty()
		{
			// Try again, but with `entity_ref` and `thread_ref` swapped:
			std::swap(entity_ref, thread_ref);

			result = parse_thread_details(thread_ref, entity_ref, opt_descriptor);
		}

		if (std::get<0>(result))
		{
			return { instruction, std::move(std::get<0>(result)), std::get<1>(result) };
		}

		return { instruction, std::nullopt, false };
	}

	std::tuple<std::optional<EntityThreadInstruction>, bool>
	parse_thread_details
	(
		std::string_view combined_expr,

		const EntityDescriptor* opt_descriptor
	)
	{
		if (combined_expr.empty())
		{
			return { std::nullopt, false };
		}

		if (auto [entity_ref, thread_ref, entity_ref_length_parsed] = util::parse_member_reference(combined_expr, true); !entity_ref.empty())
		{
			return parse_thread_details(thread_ref, entity_ref, opt_descriptor);
		}

		return parse_thread_details(combined_expr, {}, nullptr);
	}

	std::tuple<std::optional<EntityThreadInstruction>, bool>
	parse_thread_details
	(
		std::string_view thread_ref,
		std::string_view entity_ref,

		const EntityDescriptor* opt_descriptor
	)
	{
		using namespace engine::literals;

		EntityThreadInstruction thread_details;

		bool accessor_used = false;

		if (!entity_ref.empty())
		{
			if (auto target_entity = EntityTarget::parse(entity_ref))
			{
				thread_details.target_entity = std::move(*target_entity);
			}
		}

		const auto thread_accessor_content = util::parse_command(thread_ref, false);
		const auto& thread_accessor = std::get<0>(thread_accessor_content);

		if (thread_accessor.empty())
		{
			if (!thread_ref.empty())
			{
				// No accessor found; use the thread-reference directly as an ID.
				thread_details.thread_id = hash(thread_ref);
			}
		}
		else
		{
			accessor_used = true;

			const auto thread_accessor_id = hash(thread_accessor).value();
			const auto& thread_access_identifier = std::get<1>(thread_accessor_content);

			switch (thread_accessor_id)
			{
				case "thread"_hs:
				{
					thread_details.thread_id = hash(thread_access_identifier);

					break;
				}
				case "thread_index"_hs:
				{
					assert(opt_descriptor);

					if (opt_descriptor)
					{
						const auto thread_index = util::from_string<EntityThreadIndex>(thread_access_identifier);

						assert(thread_index);

						if (thread_index)
						{
							thread_details.thread_id = opt_descriptor->get_thread_id(*thread_index);

							//assert(thread_details.thread_id);
						}
					}

					break;
				}

				default:
				{
					// Invalid/unsupported accessor.
					return { std::nullopt, false }; // true
				}
			}
		}

		return { thread_details, accessor_used };
	}

	bool parse_implicit_component_settings(const util::json& data)
	{
		if (const auto implicit_components = util::find_any(data, "implicit_components", "import_components", "component_imports"); implicit_components != data.end())
		{
			if (implicit_components->is_boolean())
			{
				return implicit_components->get<bool>();
			}
		}

		return true;
	}

	const EntityState* process_state
	(
		EntityDescriptor& descriptor,
		EntityStateCollection& states_out,
		std::string_view state_path_raw, // const std::string&
		const std::filesystem::path& base_path,
		const MetaParsingContext& opt_parsing_context,
		const EntityFactoryContext* opt_factory_context,
		std::string_view state_name
	)
	{
		auto [state_base_path, state_path, state_data] = load_state_data(state_path_raw, base_path, opt_factory_context);

		std::string state_name_from_path;

		const EntityState* state_out = nullptr;

		if (state_name.empty())
		{
			state_name_from_path = resolve_state_name(state_data, state_path);

			state_name = state_name_from_path;
		}

		state_out = process_state(descriptor, states_out, state_data, state_name, state_base_path, opt_parsing_context, opt_factory_context);

		assert((!state_out) || (state_out->name.has_value()));

		return state_out;
	}

	std::size_t merge_state_list
	(
		EntityDescriptor& descriptor,
		EntityStateCollection& states_out,
		EntityState& state,

		const util::json& data,
		const std::filesystem::path& base_path,
		const MetaParsingContext& opt_parsing_context,
		const EntityFactoryContext* opt_factory_context
	)
	{
		std::size_t count = 0;

		util::json_for_each
		(
			data,

			[&base_path, opt_parsing_context, opt_factory_context, &descriptor, &states_out, &state, &count](const util::json& content)
			{
				switch (content.type())
				{
					case util::json::value_t::string:
					{
						const auto state_path_raw = content.get<std::string>();

						auto [state_base_path, state_path, state_data] = load_state_data(state_path_raw, base_path, opt_factory_context);

						if (!state_data.empty())
						{
							if (!state.name)
							{
								const auto default_state_name = default_state_name_from_path(state_path);

								state.name = hash(default_state_name);
							}

							if (process_state(descriptor, states_out, state, state_data, state_base_path, opt_parsing_context, opt_factory_context))
							{
								count++;
							}
						}

						break;
					}

					default:
						if (process_state(descriptor, states_out, state, content, base_path, opt_parsing_context, opt_factory_context))
						{
							count++;
						}

						break;
				}
			}
		);

		return count;
	}

	const EntityState* process_state
	(
		EntityDescriptor& descriptor,
		EntityStateCollection& states_out,
		const util::json& data,
		std::string_view state_name,
		const std::filesystem::path& base_path,
		const MetaParsingContext& opt_parsing_context,
		const EntityFactoryContext* opt_factory_context
	)
	{
		std::optional<EntityStateID> state_id = std::nullopt;

		if (!state_name.empty())
		{
			state_id = hash(state_name);

			// TODO: Implement multi-entity state descriptions. (use of `descriptor`)
			if (auto existing = states_out.get_state(descriptor, *state_id))
			{
				print_warn("Overriding existing definition of state: \"{}\"", state_name);

				// Initialize `existing` back to its default state.
				*existing = {};

				// Re-assign the name/ID of this state.
				existing->name = state_id;

				// Execute the main processing routine.
				auto result = process_state(descriptor, states_out, *existing, data, base_path, opt_parsing_context, opt_factory_context);

				assert(result);

				return existing;
			}
		}

		EntityState state;

		// Assign the state's name to the ID we resolved.
		state.name = state_id;

		const bool has_state_name = (!state_name.empty());

		auto implicit_components_added = std::size_t {};

		if (has_state_name)
		{
			const auto implicit_components = parse_implicit_component_settings(data);

			if (implicit_components)
			{
				process_state_add_implicit_component_from_name
				(
					descriptor, state, state_name,
					opt_parsing_context, opt_factory_context
				);
			}
		}

		if (!process_state(descriptor, states_out, state, data, base_path, opt_parsing_context, opt_factory_context))
		{
			if (implicit_components_added == 0)
			{
				return {};
			}
		}

		if (has_state_name)
		{
			process_state_default_threads
			(
				descriptor, state, state_name,
				&base_path, opt_parsing_context, opt_factory_context
			);
		}

		// Check if `state`'s name has been changed from `state_id`.
		// 
		// NOTE: Control paths from the path-based overload of `process_state`
		// should always provide the state's name ahead of time, so that shouldn't trigger this.
		if (state.name != state_id)
		{
			// TODO: Implement multi-entity state descriptions. (use of `descriptor`)
			if (auto existing = states_out.get_state(descriptor, state_id))
			{
				// If this additional check for an existing state is triggered, it means a
				// `merge` operation caused the name of a state to be defined,
				// rather than during the initial declaration.
				print_warn("Existing state named \"{}\" (#{}) detected after initial safety checks. -- Perhaps you merged an unnamed state with a named one? (Continuing anyway; replacing existing)", state_name, *state_id);

				*existing = std::move(state);

				return existing;
			}
			else
			{
				// This path is only reached if the state's name has changed,
				// but there's no existing state with the new name. (No conflict)

				if (state.name && state_id)
				{
					print_warn("State name (#{}) differs from initially resolved value (#{}: \"{}\") -- Maybe caused by a merge operation? (Continuing anyway; no known name conflicts detected.)", *state.name, *state_id, state_name);
				}
				else
				{
					print_warn("State name inherited from initial merge operation. (#{}) (Continuing anyway; no known name conflicts detected)", *state.name);
				}
			}
		}

		//descriptor.set_state(std::move(state));
		//descriptor.states.emplace_back(std::move(state));

		// TODO: Implement multi-entity state descriptions.
		auto storage_result = descriptor.allocate(std::move(state));

		const auto state_ptr = &(storage_result.get(descriptor));

		// Store `state` inside of `states_out`.
		const auto& result = states_out.data.emplace_back(std::move(storage_result));

		return state_ptr;
	}

	bool process_state
	(
		EntityDescriptor& descriptor,
		EntityStateCollection& states_out,
		EntityState& state,
		const util::json& data,
		const std::filesystem::path& base_path,
		const MetaParsingContext& opt_parsing_context,
		const EntityFactoryContext* opt_factory_context
	)
	{
		using namespace engine::literals;

		if (!state.name)
		{
			if (auto state_name = get_embedded_name(data); !state_name.empty())
			{
				state.name = hash(state_name);
			}
			else
			{
				// NOTE: Because merge operations can take place, we'll refrain
				// from spamming the user, but this warning is still valid.
				//print_warn("Missing name detected while processing state. Nameless states may cause conflicts.");
			}
		}

		// Handle merge operations first.
		// (Allows the contents of `data` to take priority over merged-in definitions):
		if (auto merge = util::find_any(data, "merge", "merge_state", "merge_states", "using"); merge != data.end())
		{
			const auto implicit_components = parse_implicit_component_settings(data);

			if (implicit_components)
			{
				process_state_local_copy_components(descriptor, state, *merge, opt_parsing_context);
			}

			// NOTE: Recursion by proxy.
			merge_state_list(descriptor, states_out, state, *merge, base_path, opt_parsing_context, opt_factory_context);
		}

		// Handle embedded imports of other states before any further operations.
		// (Allows imports in this scope to take priority over imports-within-imports):
		if (auto imports = util::find_any(data, "import", "imports", "state", "states"); imports != data.end())
		{
			// NOTE: Recursion by proxy.
			process_state_list(descriptor, states_out, *imports, base_path, opt_parsing_context, opt_factory_context);
		}

		if (auto persist = util::find_any(data, "persist", "share", "shared", "modify", "="); persist != data.end())
		{
			process_component_list(descriptor, state.components.persist, *persist, {}, opt_parsing_context, true, true, true);
		}

		if (auto add = util::find_any(data, "add", "+"); add != data.end())
		{
			process_component_list(descriptor, state.components.add, *add, {}, opt_parsing_context, true, true, true);
		}

		if (auto removal_list = util::find_any(data, "remove", "-", "~"); removal_list != data.end())
		{
			state.build_removals(descriptor, *removal_list, opt_parsing_context);
		}
		
		if (auto frozen_list = util::find_any(data, "frozen", "freeze", "exclude", "%", "^"); frozen_list != data.end())
		{
			state.build_frozen(descriptor, *frozen_list, opt_parsing_context);
		}

		if (auto storage_list = util::find_any(data, "store", "storage", "include", "temp", "temporary", "#"); storage_list != data.end())
		{
			state.build_storage(descriptor, *storage_list, opt_parsing_context);
		}

		if (auto isolated = util::find_any(data, "local", "local_storage", "isolate", "isolated"); isolated != data.end())
		{
			process_state_isolated_components(descriptor, state, *isolated, opt_parsing_context);
		}

		if (auto local_copy = util::find_any(data, "local_copy", "copy"); local_copy != data.end())
		{
			process_state_local_copy_components(descriptor, state, *local_copy, opt_parsing_context);
		}

		if (auto init_copy = util::find_any(data, "init_copy", "local_modify", "copy_once", "clone"); init_copy != data.end())
		{
			process_state_init_copy_components(descriptor, state, *init_copy, opt_parsing_context);
		}

		if (auto time_data = util::find_any(data, "timer", "wait", "delay"); time_data != data.end())
		{
			state.activation_delay = parse_time_duration(*time_data);
		}

		if (auto threads = util::find_any(data, "do", "threads", "execute", "on_enter"); threads != data.end())
		{
			process_thread_list
			(
				descriptor, *threads,
				
				[&state](EntityThreadIndex thread_index, EntityThreadCount threads_processed)
				{
					state.immediate_threads.emplace_back(thread_index, threads_processed);
				},

				&base_path, opt_parsing_context, opt_factory_context
			);
		}

		if (auto rules = util::find_any(data, "rule", "rules", "trigger", "triggers"); rules != data.end())
		{
			process_state_rule_list(descriptor, state, *rules, &states_out, &base_path, opt_parsing_context, opt_factory_context);
		}

		return true;
	}

	std::size_t process_state_list
	(
		EntityDescriptor& descriptor,
		EntityStateCollection& states_out,
		const util::json& data,
		const std::filesystem::path& base_path,
		const MetaParsingContext& opt_parsing_context,
		const EntityFactoryContext* opt_factory_context
	)
	{
		std::size_t count = 0;

		switch (data.type())
		{
			case util::json::value_t::object:
			{
				for (const auto& [state_name, state_entry] : data.items())
				{
					switch (state_entry.type())
					{
						case util::json::value_t::object:
						{
							if (process_state(descriptor, states_out, state_entry, state_name, base_path, opt_parsing_context, opt_factory_context))
							{
								count++;
							}

							break;
						}
						case util::json::value_t::string:
						{
							const auto state_path_raw = state_entry.get<std::string>();

							if (process_state(descriptor, states_out, state_path_raw, base_path, opt_parsing_context, opt_factory_context, state_name))
							{
								count++;
							}

							break;
						}
					}
				}

				break;
			}
			default:
			{
				util::json_for_each(data, [&base_path, &states_out, opt_parsing_context, opt_factory_context, &descriptor, &count](const util::json& state_entry)
				{
					switch (state_entry.type())
					{
						case util::json::value_t::object:
						{
							// NOTE: Embedded state definitions must have a `name` field.
							const auto state_name = util::get_value<std::string>(state_entry, "name");

							if (process_state(descriptor, states_out, state_entry, state_name, base_path, opt_parsing_context, opt_factory_context))
							{
								count++;
							}

							break;
						}
						case util::json::value_t::string:
						{
							const auto state_path_raw = state_entry.get<std::string>();

							if (process_state(descriptor, states_out, state_path_raw, base_path, opt_parsing_context, opt_factory_context))
							{
								count++;
							}

							break;
						}
					}
				});

				break;
			}
		}

		return count;
	}

	std::size_t process_state_isolated_components
	(
		EntityDescriptor& descriptor,
		EntityState& state,
		const util::json& isolated,
		const MetaParsingContext& opt_parsing_context
	)
	{
		return process_and_inspect_component_list
		(
			descriptor,
			state.components.add,

			isolated,

			[&descriptor, &state, &opt_parsing_context](std::string_view component_name)
			{
				if (!state.process_type_list_entry(descriptor, state.components.freeze, component_name, true, opt_parsing_context)) // false
				{
					print_warn("Failed to process embedded `freeze` entry from isolation data.");

					return false;
				}

				if (!state.process_type_list_entry(descriptor, state.components.store, component_name, true, opt_parsing_context)) // false
				{
					print_warn("Failed to process embedded `store` entry from isolation data.");

					return false;
				}

				return true;
			},

			false,
			{},
			opt_parsing_context
		);
	}

	std::size_t process_state_local_copy_components
	(
		EntityDescriptor& descriptor,
		EntityState& state,
		const util::json& local_copy,
		const MetaParsingContext& opt_parsing_context
	)
	{
		const auto build_result = state.build_local_copy(descriptor, local_copy, opt_parsing_context);

		const auto components_processed = process_component_list
		(
			descriptor, state.components.add, local_copy,
			{
				.allow_default_construction             = false,
				.allow_forwarding_fields_to_constructor = false,
				.force_field_assignment                 = true
			},

			opt_parsing_context,

			true, false
		);

		//assert(components_processed == build_result);

		return build_result;
	}

	std::size_t process_state_init_copy_components
	(
		EntityDescriptor& descriptor,
		EntityState& state,
		const util::json& init_copy,
		const MetaParsingContext& opt_parsing_context
	)
	{
		const auto build_result = state.build_init_copy(descriptor, init_copy, opt_parsing_context);

		const auto components_processed = process_component_list
		(
			descriptor, state.components.add, init_copy,
			{
				.allow_default_construction             = false,
				.allow_forwarding_fields_to_constructor = false,
				.force_field_assignment                 = true
			},

			opt_parsing_context,

			true, false
		);

		//assert(components_processed == build_result);

		return build_result;
	}

	std::size_t process_state_rule
	(
		EntityDescriptor& descriptor,
		EntityState& state,
		MetaTypeID type_name_id,
		const util::json& content,
		
		std::optional<EventTriggerCondition> condition,

		EntityStateCollection* opt_states_out,
		const std::filesystem::path* opt_base_path,
		const MetaParsingContext& opt_parsing_context,
		const EntityFactoryContext* opt_factory_context,

		bool allow_inline_import
	)
	{
		using CommandContent = MetaTypeDescriptor; // EntityStateCommandAction::CommandContent;

		std::size_t count = 0;

		EntityTarget::TargetType target = EntityTarget::SelfTarget {};
		std::optional<Timer::Duration> delay = std::nullopt;

		// NOTE: Modified below, used in `process_transition_rule` and co.
		bool forward_condition = true;

		if (condition)
		{
			// Handle special-case scenarios: direct usage of `EventTriggerTrueCondition` and `EventTriggerFalseCondition`.
			switch (condition->index())
			{
				case util::variant_index<EventTriggerConditionVariant, EventTriggerTrueCondition>():
					//print("Pure 'true-condition' detected; generating rule without condition.");

					// Don't bother forwarding this condition since it's guaranteed to always report true.
					forward_condition = false;

					break;
				case util::variant_index<EventTriggerConditionVariant, EventTriggerFalseCondition>():
					print_warn("Pure 'false-condition' found outside of compound condition; ignoring rule generation.");

					// Exit early; trigger condition will never be met.
					return count; // 0;
			}
		}

		auto& rules_out = state.rules[type_name_id];

		auto process_rule = [&descriptor, &count, &rules_out, forward_condition, &condition, &delay, &target](auto&& action)
		{
			std::optional<EntityDescriptorShared<EventTriggerCondition>> condition_out = std::nullopt;

			if (forward_condition && condition)
			{
				condition_out = descriptor.allocate<EventTriggerCondition>(*condition); // std::move(*condition)
			}

			auto& rule = rules_out.emplace_back
			(
				std::move(condition_out),

				delay, // std::move(delay),
				EntityTarget(target), // std::move(target)

				EntityStateAction(std::move(action))
			);

			count++;

			return &rule;
		};

		auto process_transition_rule = [&](const auto& next_state_raw) -> EntityStateRule*
		{
			if (next_state_raw.empty())
			{
				return nullptr;
			}

			// TODO: Look into tracking imports/inline-imports.
			const auto inline_import = process_state_inline_import(descriptor, opt_states_out, next_state_raw, opt_base_path, opt_parsing_context, opt_factory_context);

			StringHash next_state_id = (inline_import) // EntityStateHash
				? *inline_import->name
				: hash(next_state_raw)
			;

			return process_rule
			(
				EntityStateTransitionAction
				{
					.state_name = next_state_id
				}
			);
		};

		auto process_command = [&descriptor, &process_rule](CommandContent& content) // CommandContent&& content
		{
			return process_rule
			(
				EntityStateCommandAction
				{
					descriptor.shared_storage.get_index_unsafe(content) // std::move(content)
				}
			);
		};

		auto init_empty_command = [&descriptor, opt_parsing_context](std::string_view command_name) -> CommandContent*
		{
			if (command_name.empty())
			{
				return {};
			}

			const auto command_name_id = hash(command_name);
			const auto command_type = resolve(command_name_id);

			if (command_type)
			{
				return &(descriptor.generate_empty_command(command_type));
			}
			else
			{
				if (const auto opt_type_context = opt_parsing_context.get_type_context())
				{
					if (auto resolved_command_type = opt_type_context->get_command_type_from_alias(command_name))
					{
						return &(descriptor.generate_empty_command(resolved_command_type));
					}
				}

				print_warn("Unable to resolve command name: {} (#{})", command_name, command_name_id.value());
			}

			return {};
		};

		auto process_command_rule_from_csv = [&descriptor, &opt_parsing_context, &init_empty_command, &process_command](std::string_view command_name, std::string_view args, bool resolve_values=true) -> EntityStateRule*
		{
			if (args.empty())
			{
				return {};
			}

			if (auto command_content = init_empty_command(command_name))
			{
				command_content->set_variables
				(
					args,
					
					{
						.context = opt_parsing_context,
						.storage = &(descriptor.get_shared_storage()),

						.resolve_symbol = resolve_values,
						.allow_member_references = resolve_values
					},
					
					command_arg_offset
				);

				return process_command(*command_content); // std::move(*command_content)
			}

			return {};
		};

		auto process_command_rule_from_expr = [&process_command_rule_from_csv](const auto& command_expr) -> EntityStateRule*
		{
			auto [command_name, command_content, trailing_expr, is_string_content, command_parsed_length] = util::parse_command(command_expr);

			return process_command_rule_from_csv(command_name, command_content, !is_string_content);
		};

		auto process_command_rule_from_object = [&descriptor, &opt_parsing_context, &init_empty_command, &process_command](std::string_view command_name, const util::json& command_data) -> EntityStateRule*
		{
			if (auto command_content = init_empty_command(command_name))
			{
				command_content->set_variables
				(
					command_data,
					{
						.context = opt_parsing_context,
						.storage = &(descriptor.get_shared_storage()),
						
						.fallback_to_component_reference  = false,

						.allow_member_references          = true,
						.allow_entity_indirection         = true,

						.resolve_command_aliases          = true
					},

					command_arg_offset
				);

				return process_command(*command_content); // std::move(*command_content)
			}

			return {};
		};

		auto for_each_command = [&process_command_rule_from_object, &process_command_rule_from_expr, &process_command_rule_from_csv](const util::json& command)
		{
			util::json_for_each<util::json::value_t::string, util::json::value_t::object>
			(
				command,

				[&process_command_rule_from_object, &process_command_rule_from_expr, &process_command_rule_from_csv](const util::json& command_content)
				{
					switch (command_content.type())
					{
						case util::json::value_t::object:
						{
							if (auto command_name_it = util::find_any(command_content, "command", "name", "type"); command_name_it != command_content.end())
							{
								const auto command_name = command_name_it->get<std::string>();

								process_command_rule_from_object(command_name, command_content);
							}
							else
							{
								// TODO: Refactor to share code with `enumerate_map_filtered_ex` section.
								for (const auto& [command_name, embedded_content] : command_content.items())
								{
									switch (embedded_content.type())
									{
										case util::json::value_t::object:
										{
											process_command_rule_from_object(command_name, embedded_content);

											break;
										}

										case util::json::value_t::string:
										{
											auto string_data = embedded_content.get<std::string>();

											process_command_rule_from_csv(command_name, string_data);

											break;
										}
									}
								}
							}

							break;
						}

						case util::json::value_t::string:
							process_command_rule_from_expr(command_content.get<std::string>());

							break;
					}
				}
			);
		};

		switch (content.type())
		{
			case util::json::value_t::string:
			{
				process_transition_rule(content.get<std::string>());

				break;
			}

			case util::json::value_t::array:
			{
				for_each_command(content);

				break;
			}

			case util::json::value_t::object:
			{
				if (auto target_data = util::find_any(content, "target"); target_data != content.end())
				{
					target = parse_target_type(*target_data);
				}

				if (auto time_data = util::find_any(content, "timer", "wait", "delay"); time_data != content.end())
				{
					delay = parse_time_duration(*time_data);
				}

				if (auto next_state = util::find_any(content, "state", "next", "next_state"); next_state != content.end())
				{
					process_transition_rule(next_state->get<std::string>());
				}

				if (auto update = util::find_any(content, "update", "updates", "components", "change"); update != content.end())
				{
					util::json_for_each
					(
						*update,

						[&opt_parsing_context, &process_rule, &descriptor, &target](const util::json& update_entry)
						{
							auto action = EntityStateUpdateAction { target };

							process_update_action(descriptor, action, update_entry, opt_parsing_context);

							process_rule(std::move(action));
						}
					);
				}

				if (auto threads = util::find_any(content, "do", "threads", "execute", "on_enter"); threads != content.end())
				{
					process_thread_list
					(
						descriptor, *threads,
						
						[&process_rule](EntityThreadIndex thread_index, EntityThreadCount threads_processed)
						{
							process_rule
							(
								EntityThreadSpawnAction
								{
									EntityThreadRange
									{
										thread_index,
										threads_processed
									}
								}
							);
						},

						opt_base_path, opt_parsing_context, opt_factory_context
					);
				}

				// TODO: Implement sections for thread control-flow actions.

				if (auto command = util::find_any(content, "command", "commands", "generate"); command != content.end())
				{
					for_each_command(*command);
				}
				
				std::optional<EntityStateUpdateAction> update_action = std::nullopt;

				util::enumerate_map_filtered_ex
				(
					content.items(),

					[](auto&& value) { return hash(std::forward<decltype(value)>(value)); },

					[&opt_parsing_context, &process_rule, &descriptor, &target, &process_command_rule_from_object, &process_command_rule_from_csv, &update_action](const auto& element_name, const auto& content)
					{
						if (auto [component_name, allow_entry_update, constructor_arg_count] = parse_component_declaration(element_name); !component_name.empty())
						{
							auto component_type = MetaType {};

							if (const auto opt_type_context = opt_parsing_context.get_type_context())
							{
								if (component_name.ends_with("Component"))
								{
									component_type = opt_type_context->get_component_type(component_name);
								}
								else
								{
									component_type = opt_type_context->get_component_type_from_alias(component_name);
								}
							}
							else
							{
								if (component_name.ends_with("Component"))
								{
									component_type = resolve(hash(component_name).value());
								}
							}

							if (component_type)
							{
								if (!update_action)
								{
									update_action = EntityStateUpdateAction { target };
								}

								auto result = process_component
								(
									descriptor,
									update_action->updated_components,

									component_type,
									allow_entry_update,
									constructor_arg_count,

									&content,

									{}, // { .force_field_assignment = true },
									opt_parsing_context,

									true, true, true
								);

								if (result)
								{
									return;
								}
							}
						}

						switch (content.type())
						{
							case util::json::value_t::array:
							case util::json::value_t::object:
							{
								process_command_rule_from_object(element_name, content);

								break;
							}

							case util::json::value_t::string:
							{
								auto string_data = content.get<std::string>();

								process_command_rule_from_csv(element_name, string_data);

								break;
							}
						}
					},

					// Ignore these keys (see above):
					"trigger", "triggers", "condition", "conditions",

					"target",
					"timer", "wait", "delay",
					"state", "next", "next_state",
					"update", "updates", "components", "change",
					"do", "threads", "execute", "on_enter",
					"command", "commands", "generate"
				);

				if (update_action)
				{
					//assert(!update_action->updated_components.type_definitions.empty());

					if (!update_action->updated_components.type_definitions.empty())
					{
						process_rule(std::move(*update_action));
					}
				}

				break;
			}
		}

		return count;
	}

	std::size_t process_trigger_expression
	(
		EntityDescriptor& descriptor,
		EntityState& state,

		std::string_view trigger_condition_expr,
		const util::json& content,

		EntityStateCollection* opt_states_out,
		const std::filesystem::path* opt_base_path,
		const MetaParsingContext& opt_parsing_context,
		const EntityFactoryContext* opt_factory_context,
		bool allow_inline_import
	)
	{
		std::size_t number_processed = 0;

		process_trigger_expression
		(
			descriptor,

			trigger_condition_expr,

			[&descriptor, &state, &content, opt_states_out, opt_base_path, opt_parsing_context, opt_factory_context, allow_inline_import, &number_processed]
			(
				MetaTypeID type_name_id,
				std::optional<EventTriggerCondition> condition
			)
			{
				const auto result = process_state_rule
				(
					descriptor,
					state,
					type_name_id, content, condition,
					opt_states_out, opt_base_path,
					opt_parsing_context, opt_factory_context,
					allow_inline_import
				);
				
				number_processed += result;

				return result;
			},

			false, // true,
			opt_parsing_context
		);

		// NOTE: Updated automatically in `process_rule` subroutine.
		return number_processed;
	}

	std::size_t process_state_rule_list
	(
		EntityDescriptor& descriptor,
		EntityState& state,
		const util::json& rules,

		EntityStateCollection* opt_states_out,
		const std::filesystem::path* opt_base_path,
		const MetaParsingContext& opt_parsing_context,
		const EntityFactoryContext* opt_factory_context,
		bool allow_inline_import
	)
	{
		std::size_t count = 0;

		const auto collection_has_named_members = (rules.is_object());

		constexpr auto trigger_symbols = std::array { "trigger", "triggers", "condition", "conditions" };

		auto get_trigger_object = [&](const util::json& content)
		{
			return std::apply
			(
				[&](auto&&... values)
				{
					return util::find_any(content, std::forward<decltype(values)>(values)...);
				},

				trigger_symbols
			);
		};

		// Handles the "rules": [{}, {}, {}] (object array) syntax.
		auto from_json_object = [&](const util::json& content)
		{
			if (const auto trigger_decl = get_trigger_object(content); trigger_decl != content.end())
			{
				util::json_for_each(*trigger_decl, [&](const util::json& decl)
				{
					// Embedded (i.e. regular) trigger condition.
					const auto trigger_condition_expr = decl.get<std::string>();

					count += process_trigger_expression
					(
						descriptor,
						state, trigger_condition_expr, content,
						opt_states_out, opt_base_path,
						opt_parsing_context, opt_factory_context,
						allow_inline_import
					);
				});
			}
		};

		// Handles the '"TriggerCondition": { actions }' syntax.
		auto from_simplified = [&](const auto& trigger_condition_expr, const util::json& content)
		{
			count += process_trigger_expression
			(
				descriptor,
				state, trigger_condition_expr, content,
				opt_states_out, opt_base_path,
				opt_parsing_context, opt_factory_context,
				allow_inline_import
			);
		};

		auto from_data = [&](const auto& declaration, const auto& content)
		{
			if ((!collection_has_named_members) || declaration.empty())
			{
				from_json_object(content);
			}
			else
			{
				// In-place trigger condition.
				const auto& trigger_condition_expr = declaration;

				from_simplified(trigger_condition_expr, content);
			}
		};

		auto for_each = [&](const util::json& rule_container)
		{
			for (const auto& proxy : rule_container.items())
			{
				const auto& declaration = proxy.key();
				const auto& content = proxy.value();

				from_data(declaration, content);
			}
		};

		// Same as `for_each`, but ignores the `trigger` field.
		auto for_each_ignore_trigger = [&](const util::json& rule_container)
		{
			return std::apply
			(
				[&](auto&&... values)
				{
					util::enumerate_map_filtered(rule_container.items(), from_data, std::forward<decltype(values)>(values)...);
				},

				trigger_symbols
			);
		};

		switch (rules.type())
		{
			case util::json::value_t::object:
			{
				// Check if a trigger was specified.
				if (auto trigger = get_trigger_object(rules); trigger != rules.end())
				{
					switch (trigger->type())
					{
						case util::json::value_t::string:
						{
							// Because we have a `trigger` field in the top-level definition,
							// and that trigger is a conditional expression, we can assume
							// that the user wanted a single rule, rather than multiple.
							const auto trigger_condition_expr = rules.get<std::string>();

							// Assume the content for this trigger is stored in `rules`.
							from_simplified(trigger_condition_expr, rules);

							break;
						}
						default:
						{
							// Similar to the above, we have a top-level `trigger` field.
							// Unlike that path, however, we do have explicitly defined content available.
							if (auto embedded_trigger = get_trigger_object(*trigger); embedded_trigger != trigger->end())
							{
								switch (embedded_trigger->type())
								{
									case util::json::value_t::string:
										from_simplified(embedded_trigger->get<std::string>(), *trigger);

										break;
									default:
										for_each(*embedded_trigger);

										break;
								}
							}

							// Since we're looking at an encapsulated `trigger` object, the rest of
							// the top-level entries (if any) should be fair game.
							for_each_ignore_trigger(rules);

							break;
						}
					}
				}
				else
				{
					for_each(rules);
				}

				break;
			}
			default:
			{
				for_each(rules);

				break;
			}
		}

		return count;
	}

	const EntityState* process_state_inline_import
	(
		EntityDescriptor& descriptor,
		EntityStateCollection* states_out,
		const std::string& command, // std::string_view
		const std::filesystem::path* base_path,
		const MetaParsingContext& opt_parsing_context,
		const EntityFactoryContext* opt_factory_context,
		bool allow_inline_import
	)
	{
		using namespace engine::literals;

		if (!allow_inline_import)
		{
			return nullptr;
		}

		if (!states_out)
		{
			return nullptr;
		}

		if (!base_path)
		{
			return nullptr;
		}

		auto
		[
			inline_command,
			state_path_raw,
			trailing_expr,
			is_string_content,
			inline_command_parsed_length
		] = util::parse_command(command);

		if (state_path_raw.empty())
		{
			return nullptr;
		}

		switch (hash(inline_command))
		{
			case "import"_hs:
				// NOTE: Recursion.
				return process_state
				(
					descriptor,
					*states_out,
					state_path_raw, *base_path,
					opt_parsing_context, opt_factory_context
				);
		}

		return nullptr;
	}

	EntityThreadCount process_state_default_threads
	(
		EntityDescriptor& descriptor,
		EntityState& state,
		
		std::string_view state_name,

		const std::filesystem::path* opt_base_path,
		const MetaParsingContext& opt_parsing_context,
		const EntityFactoryContext* opt_factory_context
	)
	{
		return process_default_threads
		(
			descriptor, state_name,

			[&state](EntityThreadIndex thread_index, EntityThreadCount threads_processed)
			{
				state.immediate_threads.emplace_back(thread_index, threads_processed);
			},

			opt_base_path, opt_parsing_context, opt_factory_context
		);
	}

	EntityThreadCount process_archetype_default_threads
	(
		EntityDescriptor& descriptor,

		const std::filesystem::path& archetype_path,

		const std::filesystem::path* opt_base_path,
		const MetaParsingContext& opt_parsing_context,
		const EntityFactoryContext* opt_factory_context
	)
	{
		const auto default_thread_name = archetype_path.stem();

		return process_default_threads
		(
			descriptor, default_thread_name,

			[&descriptor](EntityThreadIndex thread_index, EntityThreadCount threads_processed)
			{
				descriptor.immediate_threads.emplace_back(thread_index, threads_processed);
			},

			opt_base_path, opt_parsing_context, opt_factory_context
		);
	}

	std::size_t process_archetype_add_implicit_component_from_path
	(
		EntityDescriptor& descriptor,

		const std::filesystem::path& archetype_path,

		const std::filesystem::path* opt_base_path,
		const MetaParsingContext& opt_parsing_context,
		const EntityFactoryContext* opt_factory_context
	)
	{
		if (archetype_path.has_stem())
		{
			if (const auto archetype_local_name = archetype_path.stem(); !archetype_local_name.empty())
			{
				const auto archetype = util::json { archetype_local_name.string() };

				return process_archetype_component_list(descriptor, archetype, opt_parsing_context, true);
			}
		}

		return {};
	}

	std::size_t process_state_add_implicit_component_from_name
	(
		EntityDescriptor& descriptor,
		EntityState& state,

		std::string_view state_name,

		const MetaParsingContext& opt_parsing_context,
		const EntityFactoryContext* opt_factory_context
	)
	{
		if (state_name.empty())
		{
			return {};
		}

		const auto data = util::json { std::string { state_name } };

		return process_state_isolated_components(descriptor, state, data, opt_parsing_context);
	}

	void process_archetype
	(
		EntityDescriptor& descriptor,
		const util::json& data,
		const std::filesystem::path& base_path,
		const MetaParsingContext& opt_parsing_context,
		const EntityFactoryContext* opt_factory_context,
		bool resolve_external_modules,
		std::optional<EntityStateIndex>* opt_default_state_index_out
	)
	{
		if (resolve_external_modules)
		{
			// Handles the following:
			// "archetypes", "import", "imports", "modules", "merge", "children",
			// "implicit_components", "import_components", "component_imports"
			resolve_archetypes(descriptor, data, base_path, opt_parsing_context, opt_factory_context, opt_default_state_index_out);
		}

		if (auto components = util::find_any(data, "component", "components"); components != data.end())
		{
			process_archetype_component_list(descriptor, *components, opt_parsing_context);
		}

		if (auto states = util::find_any(data, "state", "states"); states != data.end())
		{
			process_state_list(descriptor, descriptor.states, *states, base_path, opt_parsing_context, opt_factory_context);
		}
		
		if (auto threads = util::find_any(data, "do", "threads", "execute", "on_enter"); threads != data.end())
		{
			process_thread_list
			(
				descriptor, *threads,

				[&descriptor](EntityThreadIndex thread_index, EntityThreadCount threads_processed)
				{
					descriptor.immediate_threads.emplace_back(thread_index, threads_processed);
				},

				&base_path, opt_parsing_context, opt_factory_context
			);
		}

		if (opt_default_state_index_out)
		{
			// Handles the "default_state" key.
			if (auto default_state_index = get_default_state_index(descriptor, data))
			{
				*opt_default_state_index_out = default_state_index;
			}
		}

		// Handle every other key-value pair as a component:
		util::enumerate_map_filtered_ex
		(
			data.items(),

			[](auto&& value) { return hash(std::forward<decltype(value)>(value)); },

			[&descriptor, opt_parsing_context](const auto& component_declaration, const auto& component_content)
			{
				process_component
				(
					descriptor,
					descriptor.components,

					component_declaration,
					&component_content,
					{},
					opt_parsing_context
				);
			},

			// Ignore these keys:

			// Handled in `resolve_archetypes` routine:
			"archetypes", "import", "imports", "modules", "merge",
			"implicit_components", "import_components", "component_imports",

			// Handled in this function (see above):
			"component", "components",
			"state", "states",
			"do", "threads", "execute", "on_enter",

			"default_state", // "state",

			// See below.
			"model", "models",
			"animation", "animations",
			"sequence", "sequences", "animation_sequence", "animation_sequences",
			"layer", "layers", "animation_layer", "animation_layers",
			"bone_layer", "bone_layers", "bone_animation_layer", "bone_animation_layers",

			// Handled in callback-based implementation of `process_archetype`.
			"children"
		);

		if (auto model = util::find_any(data, "model", "models"); model != data.end())
		{
			engine::load<EntityDescriptor::ModelDetails>(descriptor.model_details, *model);

			if (!descriptor.model_details.path.empty())
			{
				const auto path_raw = std::filesystem::path(descriptor.model_details.path);

				if (opt_factory_context)
				{
					descriptor.model_details.path = opt_factory_context->resolve_path(path_raw, base_path).string();
				}
				else
				{
					descriptor.model_details.path = path_raw.string();
				}
			}
		}

		if (auto animations = util::find_any(data, "animation", "animations"); animations != data.end())
		{
			process_animation_list(descriptor, descriptor.animations, *animations, opt_parsing_context);
		}

		if (auto sequences = util::find_any(data, "sequence", "sequences", "animation_sequence", "animation_sequences"); sequences != data.end())
		{
			process_animation_sequence_list(descriptor, descriptor.animations, *sequences, opt_parsing_context);
		}

		if (auto layers = util::find_any(data, "layer", "layers", "animation_layer", "animation_layers"); layers != data.end())
		{
			process_animation_layer_list(descriptor, descriptor.animations, *layers, opt_parsing_context);
		}

		if (auto bone_layers = util::find_any(data, "bone_layer", "bone_layers", "bone_animation_layer", "bone_animation_layers"); bone_layers != data.end())
		{
			process_bone_animation_layer_mapping(descriptor, descriptor.animations, *bone_layers, opt_parsing_context);
		}
	}

	void process_archetype
	(
		EntityDescriptor& descriptor,
		
		const std::filesystem::path& archetype_path,

		const std::filesystem::path& base_path,
		
		const MetaParsingContext& opt_parsing_context,
		const EntityFactoryContext* opt_factory_context,
		bool resolve_external_modules,
		
		std::optional<EntityStateIndex>* opt_default_state_index_out
	)
	{
		if (archetype_path.empty())
		{
			return;
		}

		// TODO: Optimize via caching, etc.
		auto instance = util::load_json(archetype_path);

		process_archetype
		(
			descriptor, instance, base_path,
			opt_parsing_context, opt_factory_context,
			resolve_external_modules,
			opt_default_state_index_out
		);

		process_archetype_default_threads
		(
			descriptor, archetype_path,
			&base_path, opt_parsing_context, opt_factory_context
		);

		process_archetype_add_implicit_component_from_path
		(
			descriptor, archetype_path,
			&base_path, opt_parsing_context, opt_factory_context
		);
	}

	bool resolve_archetypes
	(
		EntityDescriptor& descriptor,
		const util::json& instance,
		const std::filesystem::path& base_path,
		const MetaParsingContext& opt_parsing_context,
		const EntityFactoryContext* opt_factory_context,
		bool resolve_external_modules,
		std::optional<EntityStateIndex>* opt_default_state_index_out
	)
	{
		return resolve_archetypes
		(
			descriptor,
			instance, base_path,
			[](const auto& parent_factory, const auto& child_ctx) {},
			opt_parsing_context,
			opt_factory_context,
			resolve_external_modules,
			false,
			opt_default_state_index_out
		);
	}
}