#pragma once

#include "entity_factory_context.hpp"
#include "entity_target.hpp"
#include "entity_state_action.hpp"
#include "entity_instruction.hpp"

//#include "entity_descriptor.hpp"

#include <engine/timer.hpp>
#include <engine/meta/serial.hpp>

#include <util/json.hpp>
#include <util/parse.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <tuple>
//#include <filesystem>

namespace engine
{
	struct MetaTypeDescriptorFlags;
	struct MetaDescription;

	class EntityDescriptor;

	struct ParsingContext;

	// Offset used to account for the `source` and `target` arguments of a `Command` type.
	inline constexpr std::size_t command_arg_offset = 2;

	std::optional<Timer::Duration> parse_time_duration(const std::string& time_expr); // std::string_view
	std::optional<Timer::Duration> parse_time_duration(std::string_view time_expr);
	std::optional<Timer::Duration> parse_time_duration(const util::json& time_data);

	std::tuple
	<
		std::string_view, // type_name
		std::ptrdiff_t    // updated_offset
	>
	parse_event_type(const std::string& event_type, std::ptrdiff_t offset = 0); // std::string_view

	std::tuple
	<
		std::string_view,           // component_name
		bool,                       // allow_entry_update (In-place changes)
		std::optional<std::uint8_t> // std::optional<MetaTypeDescriptor::SmallSize>
	>
	parse_component_declaration(const std::string& component_declaration, bool default_allow_entry_update=false); // std::string_view

	std::tuple
	<
		std::string_view,                      // instruction
		std::optional<EntityThreadInstruction> // thread_details
	> parse_instruction_header
	(
		std::string_view instruction_raw,
		const EntityDescriptor* opt_descriptor=nullptr
	);

	std::optional<EntityThreadInstruction> // EntityThreadInstruction
	parse_thread_details
	(
		std::string_view combined_expr,

		const EntityDescriptor* opt_descriptor=nullptr
	);

	std::optional<EntityThreadInstruction> // EntityThreadInstruction
	parse_thread_details
	(
		std::string_view thread_ref,
		std::string_view entity_ref,

		const EntityDescriptor* opt_descriptor=nullptr
	);

	std::optional<EntityTarget> resolve_manual_target(const util::json& update_entry);

	// The return value of this function indicates how many components were processed.
	std::size_t process_update_action
	(
		EntityStateUpdateAction& action_out,
		const util::json& update_entry,
		const ParsingContext* opt_parsing_context=nullptr
	);

	std::size_t process_component_list
	(
		MetaDescription& components_out, // EntityDescriptor::TypeInfo&
		const util::json& components,

		const MetaTypeDescriptorFlags& shared_component_flags={},
		const ParsingContext* opt_parsing_context=nullptr,

		bool allow_new_entry=true,
		bool allow_default_entries=true,
		bool forward_entry_update_condition_to_flags=false,

		bool ignore_special_symbols=true
	);

	// Attempts to process the `component_declaration` and `component_content` specified,
	// adding or modifying an entry in `components_out` if successful.
	// 
	// NOTE: This overload is used internally by `process_component_list`.
	// 
	// Calling this directly can be useful when wanting to process and insert
	// individual component entries that are not necessarily stored in an encompassing JSON container.
	// 
	// e.g. top-level component entries in archetypes, which have non-component
	// names that shouldn't be enumerated by a call to `process_component_list`.
	bool process_component
	(
		MetaDescription& components_out, // EntityDescriptor::TypeInfo&

		const std::string& component_declaration, // std::string_view
		const util::json* component_content=nullptr,

		const MetaTypeDescriptorFlags& component_flags={},

		const ParsingContext* opt_parsing_context=nullptr,

		bool allow_new_entry=true,
		bool allow_default_entries=true,
		bool forward_entry_update_condition_to_flags=false,

		bool ignore_special_symbols=true
	);

	// Same as the `MetaType` overload, but handles resolution of `component_name`.
	std::optional<MetaTypeDescriptor> process_component
	(
		std::string_view component_name,
		const util::json* data,

		std::optional<std::uint8_t> constructor_arg_count=std::nullopt, // SmallSize
		const MetaTypeDescriptorFlags& component_flags={},
		const ParsingContext* opt_parsing_context=nullptr
	);

	// NOTE: The 'allow' arguments refer to construction of the underlying `MetaTypeDescriptor`.
	// To affect the component itself, use the `component_flags` argument.
	MetaTypeDescriptor process_component
	(
		MetaType component_type,
		const util::json* data=nullptr,

		std::optional<std::uint8_t> constructor_arg_count=std::nullopt, // SmallSize
		const MetaTypeDescriptorFlags& component_flags={},
		const ParsingContext* opt_parsing_context=nullptr
	);
}