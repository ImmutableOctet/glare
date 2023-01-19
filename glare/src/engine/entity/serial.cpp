#include "serial.hpp"
#include "serial_impl.hpp"
#include "entity_descriptor.hpp"

#include <engine/meta/meta.hpp>
#include <engine/meta/meta_description.hpp>
#include <engine/meta/meta_type_descriptor.hpp>
#include <engine/meta/meta_type_descriptor_flags.hpp>
#include <engine/meta/parsing_context.hpp>

#include <util/algorithm.hpp>
#include <util/string.hpp>

#include <regex>
#include <charconv>
#include <type_traits>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	std::optional<Timer::Duration> parse_time_duration(const std::string& time_expr) // std::string_view
	{
		using namespace entt::literals;

		const auto time_rgx = std::regex("([\\d\\.]+)\\s*(.*)"); // (d|h|m|s|ms|us)?

		if (std::smatch rgx_match; std::regex_search(time_expr.begin(), time_expr.end(), rgx_match, time_rgx))
		{
			const auto numeric_str = util::match_view(time_expr, rgx_match, 1);

			if (const auto number = util::from_string<Timer::DurationRaw>(numeric_str))
			{
				const auto time_symbol = util::match_view(time_expr, rgx_match, 2);

				return Timer::to_duration(*number, time_symbol);
			}
		}

		return std::nullopt;
	}

	// TODO: Optimize. (Temporary string generated due to limitation of `std::regex`)
	std::optional<Timer::Duration> parse_time_duration(std::string_view time_expr)
	{
		return parse_time_duration(std::string(time_expr));
	}

	std::optional<Timer::Duration> parse_time_duration(const util::json& time_data)
	{
		switch (time_data.type())
		{
			case util::json::value_t::string:
				if (const auto timer_expr = time_data.get<std::string>(); !timer_expr.empty())
				{
					return parse_time_duration(timer_expr);
				}

				break;

			case util::json::value_t::number_float:
				return Timer::to_duration(time_data.get<float>()); // double

			case util::json::value_t::number_integer:
			case util::json::value_t::number_unsigned:
				return Timer::Seconds(time_data.get<std::uint32_t>()); // std::uint64_t
		}

		return std::nullopt;
	}

	std::tuple<std::string_view, std::ptrdiff_t>
	parse_event_type(const std::string& event_type, std::ptrdiff_t offset) // std::string_view
	{
		const auto event_type_rgx = std::regex("([^\\s\\:\\.\\-\\>\\&\\|\\(\\)]+)\\s*([^\\s\\&\\|\\(\\)]+)?");

		if (std::smatch rgx_match; std::regex_search((event_type.begin() + offset), event_type.end(), rgx_match, event_type_rgx))
		{
			auto updated_offset = (rgx_match.suffix().first - event_type.begin());

			auto parsed_view = std::string_view
			{
				(event_type.data() + offset),
				static_cast<std::size_t>(updated_offset)
			};

			auto type_name = util::match_view(parsed_view, rgx_match, 1);

			return { type_name, updated_offset };
		}

		return {};
	}

	std::tuple<std::string_view, bool, std::optional<std::uint8_t>> // EntityFactory::SmallSize
	parse_component_declaration(const std::string& component_declaration) // std::string_view
	{
		using SmallSize = std::uint8_t;
		
		const auto component_rgx = std::regex("(\\+)?([^\\|]+)(\\|(\\d+))?");

		std::string_view name_view = {};
		bool inplace_changes = false;
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

	std::size_t process_update_action
	(
		EntityStateUpdateAction& action_out,
		const util::json& update_entry,
		const ParsingContext* opt_parsing_context
	)
	{
		const auto result = process_component_list
		(
			*action_out.updated_components,
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

	std::size_t process_component_list
	(
		MetaDescription& components_out, // EntityDescriptor::TypeInfo&
		const util::json& components,

		const MetaTypeDescriptorFlags& shared_component_flags,
		const ParsingContext* opt_parsing_context,

		bool allow_new_entry,
		bool allow_default_entries,
		bool forward_entry_update_condition_to_flags,

		bool ignore_special_symbols
	)
	{
		auto as_component =
		[
			&components_out, &shared_component_flags, &opt_parsing_context,
			allow_new_entry, allow_default_entries, forward_entry_update_condition_to_flags,
			ignore_special_symbols
		]
		(const auto& component_declaration, const util::json* component_content=nullptr) -> bool
		{
			auto [component_name, allow_entry_update, constructor_arg_count] = parse_component_declaration(component_declaration);

			if (!ignore_special_symbols)
			{
				// Used by states for `update` actions.
				if (component_name == "target")
				{
					return false;
				}
			}

			const bool force_entry_update = ((!allow_new_entry) && (!allow_default_entries));

			auto component_type = (opt_parsing_context)
				? opt_parsing_context->get_component_type(component_name)
				: meta_type_from_name(component_name)
			;

			if (!component_type)
			{
				print("Failed to resolve reflection for symbol: \"{}\"", component_name);

				return false;
			}

			auto make_component = [&]()
			{
				auto flags = shared_component_flags;

				if (forward_entry_update_condition_to_flags && allow_entry_update)
				{
					flags.force_field_assignment = true;
				}

				return process_component
				(
					component_type, component_content,
					constructor_arg_count,
					flags
				);
			};

			auto& loaded_components = components_out.type_definitions;

			// Check for an existing instance of this type ID:
			auto it = std::find_if
			(
				loaded_components.begin(), loaded_components.end(),
				[&component_type](const MetaTypeDescriptor& entry) -> bool
				{
					if (entry.type_id == component_type.id())
					{
						return true;
					}

					return false;
				}
			);

			if (it == loaded_components.end())
			{
				if ((!allow_new_entry) || (!allow_default_entries && !component_content))
				{
					return false;
				}

				// There's no existing instance, and we're allowed 
				loaded_components.emplace_back(make_component());
			}
			else if (allow_entry_update || force_entry_update)
			{
				it->set_variables(make_component());
			}
			else
			{
				if ((!allow_new_entry) || (!allow_default_entries && !component_content))
				{
					return false;
				}

				*it = make_component();
			}

			return true;
		};

		std::size_t count = 0;

		util::json_for_each(components, [&as_component, &count](const util::json& comp)
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
		});

		return count;
	}

	std::optional<MetaTypeDescriptor> process_component
	(
		std::string_view component_name,
		const util::json* data,

		std::optional<std::uint8_t> constructor_arg_count, // SmallSize

		const MetaTypeDescriptorFlags& component_flags,
		const ParsingContext* opt_parsing_context
	)
	{
		auto component_type = (opt_parsing_context)
			? opt_parsing_context->get_component_type(component_name)
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
			component_flags
		);
	}

	MetaTypeDescriptor process_component
	(
		MetaType component_type,
		const util::json* data,
		
		std::optional<std::uint8_t> constructor_arg_count, // SmallSize
		const MetaTypeDescriptorFlags& component_flags
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
					{ .resolve_component_member_references=true },
					constructor_arg_count, component_flags
				);
			}
		}

		return MetaTypeDescriptor(component_type, constructor_arg_count, component_flags);
	}

	// NOTE: Used internally by trigger-condition routines.
	static std::tuple<MetaTypeID, entt::meta_data>
	resolve_member(const entt::meta_type& type, std::string_view member_name)
	{
		if (member_name.empty())
		{
			return resolve_data_member(type, true, "entity", "target", "button", "self", "value", "name");
		}
		
		return resolve_data_member(type, true, member_name);
	}

	MetaAny process_trigger_condition_value(std::string_view compared_value_raw)
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
					.resolve_symbol     = true,
					.strip_quotes       = false,
					.fallback_to_string = false
				}
			);

			if (!compared_value)
			{
				// TODO: Add support for multiple levels of indirection. (e.g. "child(name)::RelationshipComponent::child(name)")
				// Look for (optionally entity-qualified) reference to data member of component:
				if (auto data_member = indirect_meta_data_member_from_string(compared_value_raw))
				{
					compared_value = *data_member;
				}
				else
				{
					// Look for reference to entity target:
					if (auto parse_result = EntityTarget::parse_type(compared_value_raw))
					{
						compared_value = EntityTarget { std::move(std::get<0>(*parse_result)) };
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
		const entt::meta_type& type,

		std::string_view member_name,
		std::string_view comparison_operator,
		std::string_view compared_value_raw,

		std::string_view trigger_condition_expr,

		bool embed_type_in_condition
	)
	{
		auto [member_id, member] = resolve_member(type, member_name);

		if (!member && (!member_name.empty()))
		{
			print_warn("Unable to resolve target member for trigger-condition: \"{}\"", trigger_condition_expr);

			return std::nullopt;
		}

		if (!member)
		{
			return std::nullopt;
		}
			
		if (compared_value_raw.empty())
		{
			return std::nullopt;

			// TODO: Look into idea of resolving `null` as source-entity.
			//compared_value = static_cast<Entity>(null); // null;
		}

		auto compared_value = process_trigger_condition_value(compared_value_raw);

		if (!compared_value)
		{
			return std::nullopt;
		}

		auto comparison_method = EventTriggerConditionType::get_comparison_method(comparison_operator);

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
		const entt::meta_type& type,

		std::string_view entity_ref,

		std::string_view member_name,
		std::string_view comparison_operator,
		std::string_view compared_value_raw,

		std::string_view trigger_condition_expr
	)
	{
		auto [member_id, member] = resolve_member(type, member_name);

		if (!member && (!member_name.empty()))
		{
			print_warn("Unable to resolve target member for trigger-condition: \"{}\"", trigger_condition_expr);

			return std::nullopt;
		}

		auto comparison_value = process_trigger_condition_value(compared_value_raw);

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

		auto comparison_method = EventTriggerConditionType::get_comparison_method(comparison_operator);

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

	std::tuple<std::string_view, std::optional<EntityThreadInstruction>>
	parse_instruction_header(std::string_view instruction_raw, const EntityDescriptor* opt_descriptor)
	{
		if (instruction_raw.empty())
		{
			return { instruction_raw, std::nullopt };
		}

		// Look for an initial level of indirection:
		auto [initial_ref, remainder] = util::parse_data_member_reference(instruction_raw, true);

		if (initial_ref.empty())
		{
			// There's no header, return the raw instruction data and nothing else.
			return { instruction_raw, std::nullopt };
		}

		std::string_view thread_ref;
		std::string_view entity_ref;

		std::string_view instruction;

		// Check for a second level of indirection.
		if (auto [second_ref, sub_remainder] = util::parse_data_member_reference(remainder); !second_ref.empty())
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

		return
		{
			instruction,
			parse_thread_details(thread_ref, entity_ref, opt_descriptor)
		};
	}

	std::optional<EntityThreadInstruction>
	parse_thread_details
	(
		std::string_view combined_expr,

		const EntityDescriptor* opt_descriptor
	)
	{
		if (combined_expr.empty())
		{
			return std::nullopt;
		}

		if (auto [entity_ref, thread_ref] = util::parse_data_member_reference(combined_expr, true); !entity_ref.empty())
		{
			return parse_thread_details(thread_ref, entity_ref, opt_descriptor);
		}

		return parse_thread_details(combined_expr, {}, nullptr);
	}

	std::optional<EntityThreadInstruction>
	parse_thread_details
	(
		std::string_view thread_ref,
		std::string_view entity_ref,

		const EntityDescriptor* opt_descriptor
	)
	{
		using namespace entt::literals;

		EntityThreadInstruction thread_details;

		if (!entity_ref.empty())
		{
			if (auto target_entity = EntityTarget::parse(entity_ref))
			{
				thread_details.target_entity = std::move(*target_entity);
			}
		}

		const auto thread_accessor_content = util::parse_single_argument_command(thread_ref, false);
		const auto& thread_accessor = std::get<0>(thread_accessor_content);

		if (thread_accessor.empty())
		{
			// No accessor found; use the thread-reference directly as an ID.
			thread_details.thread_id = hash(thread_ref);
		}
		else
		{
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
					assert(false);

					break;
				}
			}
		}

		return thread_details;
	}
}