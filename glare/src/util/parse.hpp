#pragma once

#include <string_view>
#include <string>
#include <tuple>

namespace util
{
	// Attempts to find the first instance of an access symbol. (`::`, `.`, `->`)
	// 
	// If an access symbol could not be found, this will return a tuple with
	// `std::string_view::npos` as the symbol position, and
	// an empty `std::string_view` as the symbol content.
	std::tuple
	<
		std::size_t,     // symbol_position
		std::string_view // symbol
	>
	find_accessor(std::string_view expr);

	std::tuple
	<
		std::string_view, // command_name
		std::string_view, // command_content
		std::string_view, // trailing_expr
		bool,             // is_string_content
		std::size_t       // parsed_length
	>
	parse_command
	(
		std::string_view command,
		
		bool allow_trailing_expr=true,
		bool allow_empty_command_name=false,
		bool allow_operator_symbol_in_command_name=false,
		bool remove_quotes_in_string_content=true
	);

	std::tuple
	<
		std::string_view, // content
		std::string_view, // trailing_expr
		bool,             // is_string_content
		std::size_t       // parsed_length
	>
	parse_value
	(
		std::string_view value,
		
		bool allow_trailing_expr,
		bool truncate_value_at_first_accessor=false,
		bool truncate_value_at_operator_symbol=false, // true,
		bool remove_quotes_in_string_content=true
	);

	std::tuple
	<
		std::string_view, // command_name_or_value
		std::string_view, // content
		std::string_view, // trailing_expr
		bool,             // is_string_content
		bool,             // is_command
		std::size_t       // parsed_length
	>
	parse_command_or_value
	(
		std::string_view command_or_value,
		
		bool allow_trailing_expr=true,
		bool allow_empty_command_name=false,
		bool truncate_value_at_first_accessor=false,
		bool truncate_command_name_at_first_accessor=false,
		bool allow_operator_symbol_in_command_name=false,
		bool truncate_value_at_operator_symbol=false, // true,
		bool remove_quotes_in_string_content=true
	);

	std::tuple
	<
		std::string_view, // operator_symbol
		std::string_view  // trailing_expr
	>
	parse_standard_operator_segment(const std::string& operator_expr, bool allow_trailing_expr=true, bool beginning_of_string_only=true);

	// TODO: Remove/refactor. (`std::regex` doesn't (currently) support `std::string_view`.
	std::tuple
	<
		std::string_view, // operator_symbol
		std::string_view  // trailing_expr
	>
	parse_standard_operator_segment(std::string_view operator_expr, bool allow_trailing_expr=true, bool beginning_of_string_only=true);

	std::tuple
	<
		std::string_view, // leading_expr
		std::string_view  // trailing_expr
	> parse_member_reference(const std::string_view value, bool allow_command_syntax=false, bool match_beginning_only=false, bool allow_trailing_command_syntax=true, bool truncate_at_first_member=false, bool truncate_value_at_first_accessor=false, bool truncate_command_name_at_first_accessor=false);

	std::tuple
	<
		std::string_view, // leading
		std::string_view, // trailing
		std::string_view  // access_operator
	>
	parse_trailing_reference(const std::string_view value, bool match_beginning_only=false, bool allow_no_accessor=false, bool limit_trailing_by_whitespace=false);
	
	std::tuple
	<
		std::string_view, // leading
		std::string_view, // trailing
		std::string_view  // access_operator
	>
	parse_trailing_reference(const std::string& value, bool match_beginning_only=false, bool allow_no_accessor=false, bool limit_trailing_by_whitespace=false);

	std::tuple
	<
		std::string_view, // scope_qualifier
		std::string_view, // variable_name
		std::string_view, // variable_type
		std::string_view, // assignment_expr
		std::string_view  // trailing_expr
	>
	parse_variable_declaration(std::string_view var_decl);

	// Finds the first `end_symbol` that doesn't already satisfy a `begin_symbol`.
	std::size_t find_scope_closing_symbol(std::string_view expr, std::string_view begin_symbol, std::string_view end_symbol, std::size_t position=0, std::string_view ignore_begin_symbol={}, std::string_view ignore_end_symbol={});

	// Finds the leading `begin_symbol` and trailing `end_symbol` of an expression.
	std::tuple<std::size_t, std::size_t> find_scope_symbols(std::string_view expr, std::string_view begin_symbol, std::string_view end_symbol, std::string_view ignore_begin_symbol={}, std::string_view ignore_end_symbol={});

	// Finds the first closing parenthesis (')') that doesn't already satisfy an enclosed expression.
	std::size_t find_closing_parenthesis(std::string_view expr, std::size_t position=0);

	// Finds the leading ('(') and trailing (')') parentheses in an expression.
    std::tuple<std::size_t, std::size_t> find_parentheses(std::string_view expr);

	std::tuple<std::size_t, std::string_view>
	find_assignment_operator(std::string_view expr, bool check_compound_operators=true, bool validate_scope_bounds=true, bool disallow_right_of_scope=false);

	std::tuple<std::size_t, std::string_view> find_operator(std::string_view expr);

	std::tuple<std::size_t, std::size_t>
	find_quotes(std::string_view str, std::string_view begin_quote_symbol="\"", std::string_view end_quote_symbol="\"");
}