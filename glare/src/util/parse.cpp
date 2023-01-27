#include "parse.hpp"
#include "string.hpp"

// NOTE: included for `std::regex` workaround.
#include <util/string.hpp>

#include <regex>

namespace util
{
	std::tuple<std::string_view, std::string_view, std::string_view, bool>
	parse_single_argument_command(const std::string& command, bool allow_trailing_expr, bool beginning_of_string_only) // std::string_view
	{
		const auto command_rgx = std::regex("([^\\s\\\"\\.\\:\\-\\>\\|\\+\\-\\~\\*\\/\\%\\<\\>\\^\\(]+)\\s*\\(\\s*(\\\"[^\\\"]*\\\"|[^\\(\\)]*)\\s*\\)\\s*(.*)");

		auto flags = (beginning_of_string_only)
			? std::regex_constants::match_continuous
			: std::regex_constants::match_default
		;

		if (std::smatch rgx_match; std::regex_search(command.begin(), command.end(), rgx_match, command_rgx, flags))
		{
			auto command_name    = match_view(command, rgx_match, 1);
			auto command_content = match_view(command, rgx_match, 2);
			auto trailing_expr   = match_view(command, rgx_match, 3);

			bool is_string_content = false;

			if (is_quoted(command_content))
			{
				command_content = unquote(command_content);
				is_string_content = true;
			}

			if (allow_trailing_expr || trailing_expr.empty())
			{
				return { command_name, command_content, trailing_expr, is_string_content };
			}
		}

		return { {}, {}, {}, false };
	}

	std::tuple<std::string_view, std::string_view, std::string_view, bool>
	parse_single_argument_command(std::string_view command, bool allow_trailing_expr, bool beginning_of_string_only)
	{
		const auto temp = std::string(command);
		auto result = parse_single_argument_command(temp, allow_trailing_expr, beginning_of_string_only);

		return
		{
			remap_string_view(temp, command, std::get<0>(result)),
			remap_string_view(temp, command, std::get<1>(result)),
			remap_string_view(temp, command, std::get<2>(result)),

			std::get<3>(result)
		};
	}

	std::tuple<std::string_view, std::string_view, std::string_view, bool, bool>
	parse_single_argument_command_or_value(const std::string& command_or_value, bool allow_trailing_expr, bool beginning_of_string_only) // std::string_view
	{
		const auto command_or_value_rgx = std::regex("((([^\\s\\\"\\.\\:\\-\\>\\|\\+\\-\\~\\*\\/\\%\\<\\>\\^\\(]+)\\s*(\\(\\s*(\\\"[^\\\"]*\\\"|[^\\(\\)]*)\\s*\\))?)|([^\\s\\\"\\(]+)|(\\\"[^\\\"]+\\\"))\\s*(.*)");

		auto flags = (beginning_of_string_only)
			? std::regex_constants::match_continuous
			: std::regex_constants::match_default
		;

		if (std::smatch rgx_match; std::regex_search(command_or_value.begin(), command_or_value.end(), rgx_match, command_or_value_rgx))
		{
			auto command_name_or_content = match_view(command_or_value, rgx_match, 1);
			auto command_content         = match_view(command_or_value, rgx_match, 5);
			auto trailing_expr           = match_view(command_or_value, rgx_match, 8);

			bool is_string_content = false;
			bool is_command = (!command_content.empty());

			std::string_view content;

			if (is_command)
			{
				if (is_quoted(command_content))
				{
					command_content = unquote(command_content);
					is_string_content = true;
				}

				content = command_name_or_content;
			}
			else
			{
				if (is_quoted(command_name_or_content))
				{
					command_name_or_content = unquote(command_name_or_content);
					is_string_content = true;
				}

				content = command_name_or_content;
			}

			if (allow_trailing_expr || trailing_expr.empty())
			{
				return { command_name_or_content, content, trailing_expr, is_string_content, is_command };
			}
		}

		return { {}, {}, {}, false, false };
	}

	std::tuple<std::string_view, std::string_view, std::string_view, bool, bool>
	parse_single_argument_command_or_value(std::string_view command_or_value, bool allow_trailing_expr, bool beginning_of_string_only)
	{
		const auto temp = std::string(command_or_value);
		auto result = parse_single_argument_command_or_value(temp, allow_trailing_expr, beginning_of_string_only);

		return
		{
			remap_string_view(temp, command_or_value, std::get<0>(result)),
			remap_string_view(temp, command_or_value, std::get<1>(result)),
			remap_string_view(temp, command_or_value, std::get<2>(result)),

			std::get<3>(result),
			std::get<4>(result)
		};
	}

	std::tuple<std::string_view, std::string_view>
	parse_standard_operator_segment(const std::string& operator_expr, bool allow_trailing_expr)
	{
		const auto operator_rgx = std::regex("\\s*([\\|\\+\\-\\~\\*\\/\\%\\<\\>\\^])\\s*(.*)");

		if (std::smatch rgx_match; std::regex_search(operator_expr.begin(), operator_expr.end(), rgx_match, operator_rgx))
		{
			auto operator_symbol = match_view(operator_expr, rgx_match, 1);
			auto trailing_expr   = match_view(operator_expr, rgx_match, 2);

			if (allow_trailing_expr || trailing_expr.empty())
			{
				return { operator_symbol, trailing_expr };
			}
		}

		return { {}, {} };
	}

	std::tuple<std::string_view, std::string_view>
	parse_standard_operator_segment(std::string_view operator_expr, bool allow_trailing_expr)
	{
		const auto temp = std::string(operator_expr);
		auto result = parse_standard_operator_segment(temp, allow_trailing_expr);

		return
		{
			remap_string_view(temp, operator_expr, std::get<0>(result)),
			remap_string_view(temp, operator_expr, std::get<1>(result))
		};
	}

	std::tuple<std::string_view, std::string_view, std::string_view, std::string_view, std::string_view, std::ptrdiff_t>
	parse_qualified_assignment_or_comparison(std::string_view condition_or_assignment, std::ptrdiff_t offset, std::string_view allowed_operators)
	{
		const auto temp = std::string(condition_or_assignment);
		auto result = parse_qualified_assignment_or_comparison(temp, offset, allowed_operators);

		return
		{
			remap_string_view(temp, condition_or_assignment, std::get<0>(result)),
			remap_string_view(temp, condition_or_assignment, std::get<1>(result)),
			remap_string_view(temp, condition_or_assignment, std::get<2>(result)),
			remap_string_view(temp, condition_or_assignment, std::get<3>(result)),
			remap_string_view(temp, condition_or_assignment, std::get<4>(result)),

			std::get<5>(result)
		};
	}

	std::tuple<std::string_view, std::string_view, std::string_view, std::string_view, std::string_view, std::ptrdiff_t>
	parse_qualified_assignment_or_comparison(const std::string& condition_or_assignment, std::ptrdiff_t offset, std::string_view allowed_operators) // std::string_view
	{
		const auto trigger_rgx = std::regex("([^\\s\\:\\.\\-\\>\\&\\|\\(\\)]+\\([^\\)]+\\)|self|this)?(\\:\\:|\\.|\\-\\>)?([^\\s\\:\\.\\-\\>\\&\\|\\(\\)]+)\\s*(\\:\\:|\\.|\\-\\>)?\\s*([^\\s\\(\\)\\|\\&]+)?\\s*(\\=|==|===|!=|!==|\\<\\>|\\<|\\<\\=|\\>|\\>\\=|\\|)\\s*([^\\=\\<\\>\\!\\&\\|]+)"); // \\s* // \\(\\)

		if (std::smatch rgx_match; std::regex_search((condition_or_assignment.begin() + offset), condition_or_assignment.end(), rgx_match, trigger_rgx))
		{
			auto updated_offset = (rgx_match.suffix().first - condition_or_assignment.begin());
			
			auto parsed_view = std::string_view
			{
				(condition_or_assignment.data() + offset),
				static_cast<std::size_t>(updated_offset)
			};

			auto comparison_or_assignment_operator = match_view(parsed_view, rgx_match, 6);

			if (!allowed_operators.empty())
			{
				if (comparison_or_assignment_operator.empty())
				{
					/*
					if (!allowed_operators.contains(' '))
					{
						return {};
					}
					*/

					return {};
				}
				else
				{
					if (!allowed_operators.contains(comparison_or_assignment_operator))
					{
						return {};
					}
				}
			}

			auto entity_ref                 = match_view(parsed_view, rgx_match, 1);
			//auto entity_access_operator   = match_view(parsed_view, rgx_match, 2);
			auto type_name                  = match_view(parsed_view, rgx_match, 3);
			//auto access_operator          = match_view(parsed_view, rgx_match, 4);
			auto member_name                = match_view(parsed_view, rgx_match, 5);
			auto compared_or_assigned_value = match_view(parsed_view, rgx_match, 7);

			return { entity_ref, type_name, member_name, comparison_or_assignment_operator, compared_or_assigned_value, updated_offset };
		}

		return {};
	}

	std::tuple<std::string_view, std::string_view> parse_data_member_reference(const std::string& value, bool allow_command_syntax, bool match_beginning_only) // std::string_view
	{
		//const auto data_member_rgx = std::regex("\\s*([^\\:\\.\\-\\>]+)\\s*(\\:\\:|\\.|\\-\\>)?\\s*([^\\s]+)");
		
		auto command_syntax = [&]() -> std::tuple<std::string_view, std::string_view>
		{
			const auto data_member_rgx = std::regex("\\s*([^\\:\\.\\-\\>]+\\((\\\"[^\\\"]*\\\"|[^\\(\\)\\:\\.\\-\\>]+)\\))\\s*(\\:\\:|\\.|\\-\\>)?\\s*([^\\s]+)");

			if (std::smatch rgx_match; std::regex_search(value.begin(), value.end(), rgx_match, data_member_rgx))
			{
				if (match_beginning_only)
				{
					if (rgx_match.position() != 0)
					{
						return {};
					}
				}

				auto type_name         = match_view(value, rgx_match, 1);
				//auto command_content = match_view(value, rgx_match, 2);
				auto access_operator   = match_view(value, rgx_match, 3);
				auto data_member_name  = match_view(value, rgx_match, 4);

				// Edge-case due to limitation in regular expression used. -- Handles scenarios
				// where single-words are split into the full-string minus the last character.
				// e.g. "self" being split into: <"sel", {}, "f">
				if (!type_name.empty() && !data_member_name.empty() && access_operator.empty())
				{
					return {};
				}

				return { type_name, data_member_name };
			}

			return {};
		};

		auto regular_syntax = [&]() -> std::tuple<std::string_view, std::string_view>
		{
			//const auto data_member_rgx = std::regex("\\s*([^\\:\\.\\-\\>]+)\\s*(\\:\\:|\\.|\\-\\>)?\\s*([^\\s]+)");
			const auto data_member_rgx = std::regex("\\s*([^\\:\\.\\-\\>\\(\\)]+)\\s*(\\:\\:|\\.|\\-\\>)?\\s*([^\\:\\.\\-\\>\\(\\)]+\\s*(\\((\\\"[^\\\"]*\\\"|[^\\(\\)]*)\\))?)"); // [^\\s]+

			if (std::smatch rgx_match; std::regex_search(value.begin(), value.end(), rgx_match, data_member_rgx))
			{
				if (match_beginning_only)
				{
					if (rgx_match.position() != 0)
					{
						return {};
					}
				}

				auto type_name        = match_view(value, rgx_match, 1);
				auto access_operator  = match_view(value, rgx_match, 2);
				auto data_member_name = match_view(value, rgx_match, 3);

				// Edge-case due to limitation in regular expression used. -- Handles scenarios
				// where single-words are split into the full-string minus the last character.
				// e.g. "self" being split into: <"sel", {}, "f">
				if (!type_name.empty() && !data_member_name.empty() && access_operator.empty())
				{
					return {};
				}

				return { type_name, data_member_name };
			}

			return {};
		};

		if (allow_command_syntax)
		{
			if (auto [type_name, data_member_name] = command_syntax(); !type_name.empty())
			{
				return { type_name, data_member_name };
			}
		}

		if (auto [type_name, data_member_name] = regular_syntax(); !type_name.empty())
		{
			return { type_name, data_member_name };
		}

		return {};
	}

	// TODO: Optimize. (Temporary string generated due to limitation of `std::regex`)
	std::tuple<std::string_view, std::string_view> parse_data_member_reference(const std::string_view value, bool allow_command_syntax, bool match_beginning_only)
	{
		const auto temp = std::string(value);
		auto result = parse_data_member_reference(temp, allow_command_syntax, match_beginning_only);

		return
		{
			remap_string_view(temp, value, std::get<0>(result)),
			remap_string_view(temp, value, std::get<1>(result))
		};
	}
}