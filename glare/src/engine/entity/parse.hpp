#pragma once

#include "entity_target.hpp"

#include <engine/timer.hpp>

#include <util/json.hpp>

#include <string>
#include <string_view>
#include <optional>

namespace engine
{
	struct MetaParsingInstructions;

	std::optional<Timer::Duration> parse_time_duration(const std::string& time_expr); // std::string_view
	std::optional<Timer::Duration> parse_time_duration(std::string_view time_expr);
	std::optional<Timer::Duration> parse_time_duration(const util::json& time_data);

	std::tuple
	<
		std::optional<EntityTarget::ParseResult>, // entity_target_parse_result (optional)
		std::string_view,                         // first_symbol (e.g. thread_name)
		std::string_view,                         // second_symbol (e.g. variable_name)
		std::string_view,                         // access_operator
		bool,                                     // first_symbol_is_command
		bool,                                     // second_symbol_is_command
		std::size_t                               // updated_offset
	>
	parse_qualified_reference
	(
		std::string_view content,
		std::size_t initial_offset=0,
		const MetaParsingInstructions* opt_parsing_instructions={},
		bool disallow_command_as_symbol=false
	);

	std::tuple
	<
		// TODO: Change this field to `std::optional<EntityTarget::ParseResult>`.
		std::string_view, // entity_ref (optional)
		std::string_view, // first_symbol (e.g. type_name)
		std::string_view, // second_symbol (e.g. member_name)
		std::string_view, // comparison_or_assignment_operator
		std::string_view, // compared_or_assigned_value
		std::size_t       // updated_offset
	>
	parse_qualified_assignment_or_comparison
	(
		std::string_view condition_or_assignment,
		std::size_t initial_offset=0,
		std::string_view allowed_operators={},
		
		bool allow_missing_operator=false,
		bool allow_empty_trailing_value=false,
		bool allow_scope_as_implied_operator=true,
		bool truncate_at_logical_operators=false,
		bool disallow_command_as_symbol=true, // false,

		const MetaParsingInstructions* opt_parsing_instructions={}
	);
}