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
		bool              // is_string_content
	> parse_single_argument_command(const std::string& command); // std::string_view

	// TODO: Remove/refactor. (`std::regex` doesn't (currently) support `std::string_view`.
	std::tuple
	<
		std::string_view, // command_name
		std::string_view, // command_content
		bool              // is_string_content
	>
	parse_single_argument_command(std::string_view command);
}