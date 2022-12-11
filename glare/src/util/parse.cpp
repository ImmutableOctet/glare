#include "parse.hpp"
#include "string.hpp"

// NOTE: included for `std::regex` workaround.
#include <util/string.hpp>

#include <regex>

namespace util
{
	std::tuple<std::string_view, std::string_view, bool>
	parse_single_argument_command(const std::string& command) // std::string_view
	{
		const auto command_rgx = std::regex("([^\\s\\(]+)\\(\\s*(\\\")?([^\\\"]*)(\\\")?\\s*\\)\\s*");

		if (std::smatch rgx_match; std::regex_search(command.begin(), command.end(), rgx_match, command_rgx))
		{
			auto command_name    = match_view(command, rgx_match, 1);
			auto command_content = match_view(command, rgx_match, 3);

			bool is_string_content = (rgx_match[2].matched && rgx_match[4].matched);

			return { command_name, command_content, is_string_content };
		}

		return { {}, {}, false };
	}

	std::tuple<std::string_view, std::string_view, bool>
	parse_single_argument_command(std::string_view command)
	{
		auto temp = std::string(command);
		auto result = parse_single_argument_command(temp);

		return
		{
			util::remap_string_view(temp, command, std::get<0>(result)),
			util::remap_string_view(temp, command, std::get<1>(result)),
			std::get<2>(result)
		};
	}
}