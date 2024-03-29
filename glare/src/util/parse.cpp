#include "parse.hpp"
#include "string.hpp"
#include "algorithm.hpp"

#include <regex>

#include <array>
#include <string_view>

namespace util
{
	std::tuple<std::size_t, std::string_view> find_accessor(std::string_view expr)
	{
		// TODO: Move access symbols to a dedicated/shared array.
		return find_first_of_ex
		(
			expr,

			std::string_view::npos,

			std::string_view("::"),
			std::string_view("."),
			std::string_view("->")
		);
	}

	std::tuple<std::size_t, std::string_view> find_last_accessor(std::string_view expr)
	{
		// TODO: Move access symbols to a dedicated/shared array.
		return find_last
		(
			expr,

			std::string_view("::"),
			std::string_view("."),
			std::string_view("->")
		);
	}

	std::tuple<std::string_view, std::string_view, std::string_view, bool, std::size_t>
	parse_command
	(
		std::string_view command,
		
		bool allow_trailing_expr,
		bool allow_empty_command_name,
		bool allow_operator_symbol_in_command_name,
		bool remove_quotes_in_string_content
	)
	{
		auto [begin_scope, end_scope] = find_parentheses(command);

		if (begin_scope == std::string_view::npos || end_scope == std::string_view::npos)
		{
			return {};
		}

		const auto command_name = trim(command.substr(0, begin_scope));
		
		if (command_name.empty())
		{
			if (!allow_empty_command_name)
			{
				return {};
			}
		}
		else
		{
			if (!allow_operator_symbol_in_command_name)
			{
				const auto [operator_position, operator_symbol] = util::find_operator(command_name);

				if (operator_position != std::string_view::npos)
				{
					return {};
				}
			}
		}

		const auto parsed_length = (end_scope + 1);
		const auto trailing_expr = trim(command.substr(parsed_length));

		if ((!allow_trailing_expr) && (!trailing_expr.empty()))
		{
			return {};
		}
		
		const auto content_index = (begin_scope + 1);
		const auto content_length = (end_scope - content_index);

		auto command_content = trim(command.substr(content_index, content_length));

		bool is_string_content = false;

		if (is_quoted(command_content))
		{
			if (remove_quotes_in_string_content)
			{
				command_content = unquote(command_content);
			}

			is_string_content = true;
		}

		return { command_name, command_content, trailing_expr, is_string_content, parsed_length };
	}

	std::tuple<std::string_view, std::string_view, bool, std::size_t>
	parse_value
	(
		std::string_view value,

		bool allow_trailing_expr,
		bool truncate_value_at_first_accessor,
		bool truncate_value_at_operator_symbol,
		bool remove_quotes_in_string_content
	)
	{
		auto content = trim(value);

		if (content.empty())
		{
			return {};
		}

		std::string_view trailing_expr;

		auto parsed_length = content.length();

		bool is_string_content = false;

		const auto [begin_quote, end_quote] = find_quotes(content);

		if (begin_quote == 0)
		{
			if (end_quote != std::string_view::npos)
			{
				parsed_length = (end_quote + 1);
				trailing_expr = trim(content.substr(parsed_length));

				if (remove_quotes_in_string_content)
				{
					// NOTE: No need for `unquote` here, since we already know
					// the 'beginning' and 'end' quote locations.
					content = content.substr(1, (end_quote - 1));
				}
				else
				{
					content = content.substr(0, end_quote);
				}

				is_string_content = true;
			}
		}
		
		if (!is_string_content)
		{
			bool content_truncated = false;

			std::size_t first_access_symbol_index = std::string_view::npos;
			std::size_t operator_symbol_index = std::string_view::npos;

			if (truncate_value_at_first_accessor)
			{
				auto accessor_result = find_accessor(content); // [first_access_symbol_index, first_access_symbol] = ...

				first_access_symbol_index = std::get<0>(accessor_result);
			}

			if (truncate_value_at_operator_symbol)
			{
				auto operator_result = find_operator(content); // [operator_symbol_index, operator_symbol] = ...

				operator_symbol_index = std::get<0>(operator_result);
			}

			if ((first_access_symbol_index != std::string_view::npos) && ((operator_symbol_index == std::string_view::npos) || (first_access_symbol_index <= operator_symbol_index)))
			{
				parsed_length = first_access_symbol_index;
				trailing_expr = trim(content.substr(first_access_symbol_index)); // + first_access_symbol.length()
				content = trim(content.substr(0, first_access_symbol_index));

				content_truncated = true;
			}

			if ((operator_symbol_index != std::string_view::npos) && ((first_access_symbol_index == std::string_view::npos) || (operator_symbol_index < first_access_symbol_index)))
			{
				parsed_length = operator_symbol_index;
				trailing_expr = trim(content.substr(operator_symbol_index)); // + operator_symbol.length()
				content = trim(content.substr(0, operator_symbol_index));

				content_truncated = true;
			}

			if (!content_truncated) // (trailing_expr.empty())
			{
				const auto whitespace = content.find_last_of(whitespace_symbols);

				if (whitespace != std::string_view::npos)
				{
					parsed_length = whitespace;
					trailing_expr = trim(content.substr(whitespace));
					content = trim(content.substr(0, whitespace));

					//content_truncated = true;
				}
			}
		}

		if ((!allow_trailing_expr) && (!trailing_expr.empty()))
		{
			return {};
		}

		if (content.empty())
		{
			return {};
		}

		return { content, trailing_expr, is_string_content, parsed_length };
	}

	std::tuple<std::string_view, std::string_view, std::string_view, bool, bool, std::size_t>
	parse_command_or_value
	(
		std::string_view command_or_value,
		
		bool allow_trailing_expr,
		bool allow_empty_command_name,
		bool truncate_value_at_first_accessor,
		bool truncate_command_name_at_first_accessor,
		bool allow_operator_symbol_in_command_name,
		bool truncate_value_at_operator_symbol,
		bool remove_quotes_in_string_content
	)
	{
		if (command_or_value.empty())
		{
			return {};
		}

		if (auto [command_name, command_content, trailing_expr, is_string_content, parsed_length] = parse_command(command_or_value, allow_trailing_expr, allow_empty_command_name, allow_operator_symbol_in_command_name, remove_quotes_in_string_content); !command_name.empty())
		{
			if (truncate_command_name_at_first_accessor)
			{
				auto [embedded_access_symbol_index, embedded_access_symbol] = find_accessor(command_name);

				if (embedded_access_symbol_index != std::string_view::npos)
				{
					const auto leading_expr = trim(command_name.substr(0, embedded_access_symbol_index));

					const auto remainder_start_index = static_cast<std::size_t>((command_name.data() + embedded_access_symbol_index) - command_or_value.data()); // + embedded_access_symbol.length()

					const auto remainder = trim(command_or_value.substr(remainder_start_index));

					return { leading_expr, leading_expr, remainder, false, false, remainder_start_index };
				}
			}

			return { command_name, command_content, trailing_expr, is_string_content, true, parsed_length };
		}

		auto [content, trailing_expr, is_string_content, parsed_length] = parse_value(command_or_value, allow_trailing_expr, truncate_value_at_first_accessor, truncate_value_at_operator_symbol, remove_quotes_in_string_content);

		return { content, content, trailing_expr, is_string_content, false, parsed_length };
	}

	std::tuple<std::string_view, std::string_view>
	parse_standard_operator_segment
	(
		std::string_view operator_expr,
		
		bool allow_trailing_expr,
		bool beginning_of_string_only
	)
	{
		if (beginning_of_string_only)
		{
			operator_expr = trim(operator_expr);
		}

		const auto [operator_position, operator_symbol] = find_operator(operator_expr);

		if (operator_position == std::string_view::npos)
		{
			return {};
		}

		if (beginning_of_string_only)
		{
			if (operator_position != 0)
			{
				return {};
			}
		}

		assert(!operator_symbol.empty());

		const auto trailing_expr = trim(operator_expr.substr(operator_position + operator_symbol.length()));

		if (!allow_trailing_expr)
		{
			if (!trailing_expr.empty())
			{
				return {};
			}
		}

		return { operator_symbol, trailing_expr };
	}

	std::tuple<std::string_view, std::string_view, std::size_t>
	parse_member_reference
	(
		std::string_view value,
		
		bool allow_command_syntax,
		bool match_beginning_only,
		bool allow_trailing_command_syntax,
		bool truncate_at_first_member,
		
		bool truncate_value_at_first_accessor,
		bool truncate_command_name_at_first_accessor
	)
	{
		auto result = parse_command_or_value
		(
			value,
			true,  // allow_trailing_expr
			false, // allow_empty_command_name
			truncate_value_at_first_accessor,
			truncate_command_name_at_first_accessor,
			false, // allow_operator_symbol_in_command_name
			true,  // truncate_value_at_operator_symbol (Needed for operator-check below)

			// TODO: Revisit whether this should be disabled.
			false // true // remove_quotes_in_string_content
		);

		const auto& command_name_or_value = std::get<0>(result);

		if (command_name_or_value.empty())
		{
			return {};
		}

		const auto leading_expr_index = static_cast<std::size_t>(command_name_or_value.data() - value.data());

		if (match_beginning_only)
		{
			const auto first_non_whitespace = value.find_first_not_of(whitespace_symbols);

			if (first_non_whitespace == std::string_view::npos)
			{
				return {};
			}

			if (leading_expr_index != first_non_whitespace) // (leading_expr_index != 0)
			{
				return {};
			}
		}

		const auto leading_expr_is_command = std::get<4>(result);

		if (leading_expr_is_command && !allow_command_syntax)
		{
			return {};
		}

		std::string_view trailing_expr_raw = std::get<2>(result);

		if (trailing_expr_raw.empty())
		{
			if (leading_expr_is_command)
			{
				return {};
			}

			//trailing_expr_raw = command_name_or_value;
		}

		std::size_t access_symbol_local_index = std::string_view::npos;
		std::string_view access_symbol = {};

		//if ((!truncate_value_at_first_accessor) || (leading_expr_is_command))
		{
			auto trailing_accessor = find_accessor(trailing_expr_raw);

			access_symbol_local_index = std::get<0>(trailing_accessor);
			access_symbol = std::get<1>(trailing_accessor);
		}

		if (access_symbol.empty())
		{
			if (leading_expr_is_command)
			{
				if (truncate_command_name_at_first_accessor)
				{
					return {};
				}
			}
			else
			{
				if (truncate_value_at_first_accessor)
				{
					return {};
				}
			}

			auto [embedded_access_index, embedded_access_symbol] = find_last_accessor(command_name_or_value);
				
			if (embedded_access_index == std::string_view::npos)
			{
				return {};
			}
			
			const auto command_name_or_value_offset = static_cast<std::size_t>(command_name_or_value.data() - value.data());
			const auto translated_access_index = (command_name_or_value_offset + embedded_access_index);

			trailing_expr_raw = value.substr(translated_access_index);

			access_symbol_local_index = 0;
			access_symbol = embedded_access_symbol;
		}
		else
		{
			// Ensure that we aren't processing an expression with operators:
			if (const auto operator_index = std::get<0>(find_operator(trailing_expr_raw)); operator_index < access_symbol_local_index)
			{
				return {};
			}
		}

		if (leading_expr_is_command)
		{
			if (access_symbol_local_index != 0)
			{
				return {};
			}
		}

		auto trailing_expr = trim(trailing_expr_raw.substr(access_symbol_local_index + access_symbol.length()));

		const auto access_symbol_value_index = static_cast<std::size_t>((trailing_expr_raw.data() + access_symbol_local_index) - value.data());

		const auto leading_expr_length = (access_symbol_value_index - leading_expr_index);

		auto leading_expr = value.substr(leading_expr_index, leading_expr_length);

		if (trailing_expr.empty())
		{
			return {};
		}

		std::size_t parsed_length = 0;

		auto member_result = parse_command_or_value
		(
			trailing_expr,
			true,  // allow_trailing_expr
			false, // allow_empty_command_name
			true,  // truncate_value_at_first_accessor,
			false, // truncate_command_name_at_first_accessor,
			false, // allow_operator_symbol_in_command_name
			true,  // truncate_value_at_operator_symbol (Needed for operator-check below)
			false // remove_quotes_in_string_content
		);

		const auto& member_length = std::get<5>(member_result);
		const auto& member_is_command = std::get<4>(member_result);

		if (truncate_at_first_member)
		{
			if (!member_length)
			{
				return {};
			}

			if (!allow_trailing_command_syntax)
			{
				if (member_is_command)
				{
					return {};
				}
			}

			trailing_expr = trim(trailing_expr.substr(0, member_length));

			// We can trust that `trailing_expr` contains an explicitly requested string,
			// and can therefore include this expression in the reported length.
			parsed_length = static_cast<std::size_t>((trailing_expr.data() + trailing_expr.length()) - value.data());
		}
		else if ((member_length) && (member_is_command))
		{
			if (!allow_trailing_command_syntax)
			{
				return {};
			}

			// TODO: Determine if we should keep this behavior.
			// 
			// Since the parsed member is a command, include it in
			// the reported length, regardless of truncation configuration.
			parsed_length = static_cast<std::size_t>((trailing_expr.data() + member_length) - value.data());
		}
		else
		{
			// `trailing_expr` is being used as a 'catch-all' for the remainder of the string;
			// only include the explicitly parsed section in the reported length.
			parsed_length = static_cast<std::size_t>((leading_expr.data() + leading_expr.length()) - value.data());
		}

		//assert(!trailing_expr.empty());

		return { leading_expr, trailing_expr, parsed_length };
	}

	// TODO: Deprecate/replace.
	std::tuple<std::string_view, std::string_view, std::string_view>
	parse_trailing_reference(const std::string& value, bool match_beginning_only, bool allow_no_accessor, bool limit_trailing_by_whitespace)
	{
		const auto access_rgx = std::regex("\\s*([^\\:\\.\\-\\>]+)?\\s*(\\:\\:|\\.|\\-\\>)?\\s*(.+)?");

		auto flags = (match_beginning_only)
			? std::regex_constants::match_continuous
			: std::regex_constants::match_default
		;

		if (std::smatch rgx_match; std::regex_search(value.begin(), value.end(), rgx_match, access_rgx, flags))
		{
			if (match_beginning_only)
			{
				if (rgx_match.position() != 0)
				{
					return {};
				}
			}

			auto leading         = match_view(value, rgx_match, 1);
			auto access_operator = match_view(value, rgx_match, 2);
			auto trailing        = match_view(value, rgx_match, 3);

			if (!allow_no_accessor && access_operator.empty())
			{
				return {};
			}

			if (leading.empty() && trailing.empty())
			{
				return {};
			}

			if (limit_trailing_by_whitespace && !trailing.empty())
			{
				auto whitespace = trailing.find_first_of(whitespace_symbols);

				if (whitespace != std::string_view::npos)
				{
					trailing = trailing.substr(0, whitespace);
				}
			}

			return { leading, trailing, access_operator };
		}

		return {};
	}

	std::tuple<std::string_view, std::string_view, std::string_view>
	parse_trailing_reference(const std::string_view value, bool match_beginning_only, bool allow_no_accessor, bool limit_trailing_by_whitespace)
	{
		const auto temp = std::string(value);
		auto result = parse_trailing_reference(temp, match_beginning_only, allow_no_accessor, limit_trailing_by_whitespace);

		return
		{
			remap_string_view(temp, value, std::get<0>(result)),
			remap_string_view(temp, value, std::get<1>(result)),
			remap_string_view(temp, value, std::get<2>(result))
		};
	}

	std::tuple
	<
		std::string_view, // scope_qualifier
		std::string_view, // variable_name
		std::string_view, // variable_type
		std::string_view, // assignment_expr
		std::string_view  // trailing_expr
	>
	parse_variable_declaration(std::string_view var_decl)
	{
		if (var_decl.empty())
		{
			return {};
		}

		auto output = std::tuple
		<
			std::string_view,
			std::string_view,
			std::string_view,
			std::string_view,
			std::string_view
		>();

		auto& [scope_qualifier, variable_name, variable_type, assignment_expr, trailing_expr] = output;

		auto remainder = var_decl;

		const auto scope_symbol_end = remainder.find_first_of(whitespace_symbols);

		scope_qualifier = remainder.substr(0, scope_symbol_end);

		if ((scope_symbol_end + 1) >= var_decl.length())
		{
			return output;
		}

		remainder = remainder.substr(scope_symbol_end + 1);

		auto variable_decl_end = remainder.find_first_of(whitespace_symbols);

		auto type_specifier_end = std::string_view::npos;

		constexpr auto assignment_operator_symbol = std::string_view { "=" };

		auto assignment_operator = remainder.find(assignment_operator_symbol);

		if ((assignment_operator != std::string_view::npos) && (variable_decl_end != std::string_view::npos))
		{
			if (assignment_operator < variable_decl_end)
			{
				variable_decl_end = assignment_operator;
			}
		}

		constexpr auto type_specifier_symbol = std::string_view { ":" };

		if (auto variable_type_begin = find_singular(remainder, type_specifier_symbol); ((variable_type_begin != std::string_view::npos) && ((assignment_operator == std::string_view::npos) || (variable_type_begin < assignment_operator))))
		{
			variable_name = util::trim(remainder.substr(0, variable_type_begin));

			if ((variable_type_begin + 1) >= remainder.length())
			{
				return output;
			}

			if (assignment_operator == std::string_view::npos)
			{
				const auto type_subset = remainder.substr((variable_type_begin + type_specifier_symbol.length()));

				if (!type_subset.empty())
				{
					const auto type_specifier_end = type_subset.find_first_of(whitespace_symbols);

					if (type_specifier_end != std::string_view::npos)
					{
						variable_type = type_subset.substr(0, type_specifier_end);
					}
					else
					{
						variable_type = type_subset;
					}
				}
			}
			else
			{
				const auto type_subset_begin = (variable_type_begin + type_specifier_symbol.length());
				const auto type_subset_length = (assignment_operator - type_subset_begin);

				const auto type_subset = remainder.substr(type_subset_begin, type_subset_length);

				if (!type_subset.empty())
				{
					variable_type = type_subset;
				}
			}

			variable_type = util::trim(variable_type);
		}
		else
		{
			if (variable_decl_end == std::string_view::npos)
			{
				variable_name = remainder;

				return output;
			}

			variable_name = util::trim(remainder.substr(0, variable_decl_end));

			if (variable_decl_end >= remainder.length())
			{
				return output;
			}

			remainder = remainder.substr(variable_decl_end);

			if (assignment_operator != std::string_view::npos)
			{
				assignment_operator -= variable_decl_end;
			}
		}

		if (assignment_operator == std::string_view::npos)
		{
			if (variable_type.empty())
			{
				trailing_expr = util::trim(remainder);
			}
			else
			{
				const auto variable_type_end = static_cast<std::size_t>((variable_type.data() + variable_type.length()) - remainder.data());

				if (variable_type_end < remainder.length())
				{
					remainder = remainder.substr(variable_type_end);

					trailing_expr = util::trim(remainder);
				}
			}
		}
		else
		{
			const auto assignment_expr_begin = (assignment_operator + assignment_operator_symbol.length());

			if (assignment_expr_begin < remainder.size())
			{
				assignment_expr = util::trim(remainder.substr(assignment_expr_begin));
			}
		}

		return output;
	}

	std::tuple
	<
		std::string_view, // key_name
		std::string_view, // value_type
		std::string_view, // trailing_expr
		bool              // expression_syntax_used
	>
	parse_key_expr_and_value_type
	(
		std::string_view key_expr,
		
		std::string_view type_specification_symbol,

		bool allow_trailing_expr,
		bool ensure_singular_type_specifier_symbol,
		bool unquote_string_literal_as_key_name,
		bool allow_key_truncation
	)
	{
		if (key_expr.empty())
		{
			return {};
		}

		auto command_result = parse_command(key_expr, true, true, false, false);

		const auto command_result_parsed_length = std::get<4>(command_result);

		auto key_name  = std::string_view {};
		auto remainder = std::string_view {};

		auto value_type_symbol = std::string_view::npos;

		const bool expression_syntax_used = (command_result_parsed_length > 0);

		auto find_symbol = [type_specification_symbol, ensure_singular_type_specifier_symbol](std::string_view str, std::size_t offset=0) -> std::size_t
		{
			if (ensure_singular_type_specifier_symbol)
			{
				return find_last_singular(str, type_specification_symbol, offset);
			}
			
			return str.find(type_specification_symbol, offset);
		};

		if (expression_syntax_used)
		{
			key_name = key_expr.substr(0, command_result_parsed_length); // std::get<1>(command_result);
			remainder = std::get<2>(command_result);
			value_type_symbol = find_symbol(remainder);
		}
		else
		{
			std::size_t type_specifier_offset = 0;

			const auto [begin_quote, end_quote] = find_quotes(key_expr);

			const auto is_quoted = (begin_quote == 0) && (end_quote != std::string_view::npos);

			if (is_quoted)
			{
				type_specifier_offset = (end_quote + 1);

				if (unquote_string_literal_as_key_name)
				{
					key_name = key_expr.substr((begin_quote + 1), (end_quote - 1));
				}
				else
				{
					key_name = key_expr.substr(begin_quote, (end_quote + 1));
				}
			}

			const auto type_specifier_position = (type_specifier_offset < key_expr.length())
				? find_symbol(key_expr, type_specifier_offset)
				: std::string_view::npos
			;

			if (type_specifier_position != std::string_view::npos)
			{
				if (!is_quoted)
				{
					key_name = key_expr.substr(0, type_specifier_position);
				}

				const auto remainder_begin = (type_specifier_position + type_specification_symbol.length() + 1);

				if (remainder_begin < key_expr.length())
				{
					remainder = key_expr.substr(type_specifier_position);

					value_type_symbol = 0;
				}
			}
			else if (is_quoted)
			{
				if (const auto remainder_begin = (end_quote + 1); remainder_begin < key_expr.length())
				{
					remainder = key_expr.substr(remainder_begin);
				}
			}
			else
			{
				if (allow_key_truncation)
				{
					if (const auto key_trailing_expr_begin = key_expr.find_first_of(whitespace_symbols); key_trailing_expr_begin != std::string_view::npos)
					{
						key_name = key_expr.substr(0, key_trailing_expr_begin);
						remainder = trim(key_expr.substr(key_trailing_expr_begin));
					}
					else
					{
						key_name = key_expr;
					}
				}
				else
				{
					key_name = key_expr;
				}
			}
		}

		if (key_name.empty())
		{
			return {};
		}

		auto value_type = std::string_view {};
		auto trailing_expr = std::string_view {};

		if (value_type_symbol != std::string_view::npos)
		{
			if (const auto value_type_begin = (value_type_symbol + type_specification_symbol.length()); value_type_begin < remainder.length())
			{
				value_type = trim(remainder.substr(value_type_begin));

				if (const auto key_trailing_expr_begin = value_type.find_first_of(whitespace_symbols); key_trailing_expr_begin != std::string_view::npos)
				{
					trailing_expr = trim(value_type.substr(key_trailing_expr_begin));
					value_type = value_type.substr(0, key_trailing_expr_begin);
				}
			}
		}
		else
		{
			trailing_expr = remainder;
		}

		if (!allow_trailing_expr)
		{
			if (!trailing_expr.empty())
			{
				return {};
			}
		}

		return { key_name, value_type, trailing_expr, expression_syntax_used };
	}

    std::size_t find_closing_parenthesis(std::string_view expr, std::size_t position)
    {
        return find_scope_closing_symbol
		(
			expr,
			
			"(", ")",
			
			position,
			
			std::array
			{
				std::pair { "[", "]" },
				std::pair { "\"", "\"" }
			}
		);
    }

    std::tuple<std::size_t, std::size_t> find_parentheses(std::string_view expr)
    {
        return find_scope_symbols
		(
			expr,

			"(", ")",
			
			std::array
			{
				std::pair { "[", "]" },
				std::pair { "\"", "\"" }
			}
		);
    }

	std::size_t find_closing_subscript(std::string_view expr, std::size_t position)
	{
		return find_scope_closing_symbol
		(
			expr,

			"[", "]",

			position,

			std::array
			{
				std::pair { "(", ")" },
				std::pair { "\"", "\"" }
			}
		);
	}

	std::tuple<std::size_t, std::size_t> find_subscript(std::string_view expr)
	{
		return find_scope_symbols
		(
			expr,

			"[", "]",

			std::array
			{
				std::pair { "(", ")" },
				std::pair { "\"", "\"" }
			}
		);
	}

	std::tuple<std::size_t, std::string_view>
	find_assignment_operator(std::string_view expr, bool check_compound_operators, bool validate_scope_bounds, bool disallow_right_of_scope)
	{
		std::size_t offset = 0;
		std::size_t assignment_symbol_index = std::string_view::npos;

		do
		{
			assignment_symbol_index = expr.find('=', offset);

			if (assignment_symbol_index == std::string_view::npos)
			{
				return { std::string_view::npos, {} };
			}

			if (validate_scope_bounds || disallow_right_of_scope)
			{
				if (auto [unrelated_scope_begin, unrelated_scope_end] = util::find_parentheses(expr); ((unrelated_scope_begin != std::string_view::npos) && (unrelated_scope_end != std::string_view::npos)))
				{
					if ((assignment_symbol_index > unrelated_scope_begin))
					{
						if (disallow_right_of_scope)
						{
							return { std::string_view::npos, {} };
						}

						if (validate_scope_bounds && (assignment_symbol_index < unrelated_scope_end))
						{
							if ((unrelated_scope_end + 1) < expr.length())
							{
								offset = (unrelated_scope_end + 1);
							}
							else
							{
								return { std::string_view::npos, {} };
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		} while (offset < expr.length());

		if (!check_compound_operators || (assignment_symbol_index == 0) || (assignment_symbol_index == std::string_view::npos))
		{
			return { assignment_symbol_index, expr.substr(assignment_symbol_index, 1) };
		}

		static_assert(std::string_view::npos == static_cast<std::size_t>(-1));

		constexpr auto special_characters = std::array { '+', '-', '*', '/', '%', '<', '>', '&', '^', '|', ':' };

		std::size_t prev_char_index;

		for (prev_char_index = (assignment_symbol_index - 1); prev_char_index != std::string_view::npos; --prev_char_index)
		{
			const auto prev_char = expr[prev_char_index];

			if (std::find(special_characters.cbegin(), special_characters.cend(), prev_char) == special_characters.cend())
			{
				break;
			}
		}

		if (prev_char_index == std::string_view::npos)
		{
			prev_char_index = 0;
		}

		const auto assignment_symbol_length = (assignment_symbol_index - prev_char_index);

		const auto last_included_char_index = (prev_char_index + 1);

		return { last_included_char_index, expr.substr(last_included_char_index, assignment_symbol_length) };
	}

	std::tuple<std::size_t, std::string_view> find_logic_operator(std::string_view expr, bool include_xor)
	{
		if (include_xor)
		{
			return util::execute_as<std::string_view>
			(
				[&expr](auto&&... operator_symbols)
				{
					/*
					// Alternative implementation (does not account for expression scope):
					return find_first_of_ex
					(
						expr,
						std::string_view::npos,
						std::forward<decltype(operator_symbols)>(operator_symbols)...
					);
					*/

					return find_first_of_unscoped
					(
						expr, util::standard_scope_symbols,
						std::forward<decltype(operator_symbols)>(operator_symbols)...
					);
				},

				"&&", "||", "^"
			);
		}
		else
		{
			return util::execute_as<std::string_view>
			(
				[&expr](auto&&... operator_symbols)
				{
					/*
					// Alternative implementation (does not account for expression scope):
					return find_first_of_ex
					(
						expr,
						std::string_view::npos,
						std::forward<decltype(operator_symbols)>(operator_symbols)...
					);
					*/

					return find_first_of_unscoped
					(
						expr, util::standard_scope_symbols,
						std::forward<decltype(operator_symbols)>(operator_symbols)...
					);
				},

				"&&", "||"
			);
		}
	}

	std::tuple<std::size_t, std::string_view> find_operator(std::string_view expr)
	{
		if (expr.empty())
		{
			return { std::string_view::npos, {} };
		}

		auto [initial_result_index, initial_result_symbol] = util::execute_as<std::string_view>
		(
			[&expr](auto&&... operator_symbols) -> std::tuple<std::size_t, std::string_view>
			{
				/*
				// Alternative implementation (does not account for expression scope):
				return find_first_of_ex
				(
					expr,
					std::string_view::npos,
					std::forward<decltype(operator_symbols)>(operator_symbols)...
				);
				*/

				return find_first_of_unscoped
				(
					expr, util::standard_scope_symbols,
					std::forward<decltype(operator_symbols)>(operator_symbols)...
				);
			},

			"&&", "||", "<<", ">>",
			"==", "!=", "<>", "<", ">",
			"+", "-", "~", "*", "/", "%", "&", "^", "|", "=", "!"
		);

		if (initial_result_index == std::string_view::npos)
		{
			return { std::string_view::npos, {} }; // { initial_result_index, initial_result_symbol };
		}

		if ((initial_result_symbol.length() == 1) || (initial_result_symbol == "<<") || (initial_result_symbol == ">>"))
		{
			auto expandable_result = [&expr](std::size_t result_index, std::size_t operator_length, auto&&... expansion_symbols) -> std::tuple<std::size_t, std::string_view>
			{
				//assert(result_index < std::numeric_limits<decltype(result_index)>::max());

				if (const auto next_index = (result_index + operator_length); next_index < expr.length())
				{
					if (((expr[next_index] == expansion_symbols) || ...))
					{
						operator_length++;
					}
				}

				return { result_index, expr.substr(result_index, operator_length) };
			};

			// Check for assignment syntax (e.g. `+=`, `-=`):
			return expandable_result(initial_result_index, initial_result_symbol.length(), '=');
		}

		return { initial_result_index, expr.substr(initial_result_index, initial_result_symbol.length()) };
	}

	std::tuple<std::size_t, std::size_t>
	find_quotes(std::string_view str, std::string_view begin_quote_symbol, std::string_view end_quote_symbol)
	{
		if (str.length() < (begin_quote_symbol.length() + end_quote_symbol.length()))
		{
			return { std::string_view::npos, std::string_view::npos };
		}

		const auto first_quote = str.find(begin_quote_symbol);

		if (first_quote == std::string_view::npos)
		{
			return { std::string_view::npos, std::string_view::npos };
		}

		std::size_t offset = (first_quote + begin_quote_symbol.length());

		while (offset < str.length())
		{
			const auto second_quote = str.find(end_quote_symbol, offset);

			if (second_quote == std::string_view::npos)
			{
				break;
			}

			const auto prev_char = str[second_quote - 1];

			if (prev_char == '\\')
			{
				offset = (second_quote + end_quote_symbol.length());

				continue;
			}

			return { first_quote, second_quote };
		}

		return { std::string_view::npos, std::string_view::npos };
	}
}