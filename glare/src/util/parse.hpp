#pragma once

#include <string_view>
#include <string>
#include <tuple>

namespace util
{
	std::tuple
	<
		std::string_view, // command_name
		std::string_view, // command_content
		std::string_view, // trailing_expr
		bool              // is_string_content
	> parse_single_argument_command(const std::string& command, bool allow_trailing_expr=true); // std::string_view

	// TODO: Remove/refactor. (`std::regex` doesn't (currently) support `std::string_view`.
	std::tuple
	<
		std::string_view, // command_name
		std::string_view, // command_content
		std::string_view, // trailing_expr
		bool              // is_string_content
	>
	parse_single_argument_command(std::string_view command, bool allow_trailing_expr=true);

	std::tuple
	<
		std::string_view, // command_name_or_value
		std::string_view, // content
		std::string_view, // trailing_expr
		bool,             // is_string_content
		bool              // is_command
	>
	parse_single_argument_command_or_value(const std::string& command_or_value, bool allow_trailing_expr=true);

	// TODO: Remove/refactor. (`std::regex` doesn't (currently) support `std::string_view`.
	std::tuple
	<
		std::string_view, // command_name_or_value
		std::string_view, // content
		std::string_view, // trailing_expr
		bool,             // is_string_content
		bool              // is_command
	>
	parse_single_argument_command_or_value(std::string_view command_or_value, bool allow_trailing_expr=true);

	std::tuple
	<
		std::string_view, // entity_ref (optional)
		std::string_view, // type_name
		std::string_view, // member_name
		std::string_view, // comparison_or_assignment_operator
		std::string_view, // compared_or_assigned_value
		std::ptrdiff_t    // updated_offset
	>
	parse_qualified_assignment_or_comparison(const std::string& condition_or_assignment, std::ptrdiff_t offset=0, std::string_view allowed_operators={});

	std::tuple<std::string_view, std::string_view, std::string_view, std::string_view, std::string_view, std::ptrdiff_t>
	parse_qualified_assignment_or_comparison(std::string_view condition_or_assignment, std::ptrdiff_t offset=0, std::string_view allowed_operators={});

	std::tuple<std::string_view, std::string_view> parse_data_member_reference(const std::string& value, bool allow_command_syntax=true, bool match_beginning_only=false);
	std::tuple<std::string_view, std::string_view> parse_data_member_reference(const std::string_view value, bool allow_command_syntax=true, bool match_beginning_only=false);
}