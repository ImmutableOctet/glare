#include "parse.hpp"
#include "string.hpp"
#include "algorithm.hpp"

#include <regex>

namespace util
{
	std::tuple<std::size_t, std::string_view> find_accessor(std::string_view expr)
	{
		return find_first_of_ex
		(
			expr,

			std::string_view::npos,

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
	parse_standard_operator_segment(const std::string& operator_expr, bool allow_trailing_expr, bool beginning_of_string_only)
	{
		const auto operator_rgx = std::regex("\\s*([\\!\\|\\+\\-\\~\\*\\/\\%\\<\\>\\^\\=]+)\\s*(.*)");

		auto flags = (beginning_of_string_only)
			? std::regex_constants::match_continuous
			: std::regex_constants::match_default
		;

		if (std::smatch rgx_match; std::regex_search(operator_expr.begin(), operator_expr.end(), rgx_match, operator_rgx, flags))
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
	parse_standard_operator_segment(std::string_view operator_expr, bool allow_trailing_expr, bool beginning_of_string_only)
	{
		const auto temp = std::string(operator_expr);
		auto result = parse_standard_operator_segment(temp, allow_trailing_expr, beginning_of_string_only);

		return
		{
			remap_string_view(temp, operator_expr, std::get<0>(result)),
			remap_string_view(temp, operator_expr, std::get<1>(result))
		};
	}

	std::tuple<std::string_view, std::string_view>
	parse_member_reference(std::string_view value, bool allow_command_syntax, bool match_beginning_only, bool allow_trailing_command_syntax, bool truncate_at_first_member, bool truncate_value_at_first_accessor, bool truncate_command_name_at_first_accessor)
	{
		auto result = parse_command_or_value(value, true, false, truncate_value_at_first_accessor, truncate_command_name_at_first_accessor);

		const auto& command_name_or_value = std::get<0>(result);

		if (command_name_or_value.empty())
		{
			return {};
		}

		const auto leading_expr_index = static_cast<std::size_t>(command_name_or_value.data() - value.data());

		if (match_beginning_only)
		{
			const auto first_non_whitespace = value.find_first_not_of(" \n\t");

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

			trailing_expr_raw = command_name_or_value;
		}

		const auto [access_symbol_local_index, access_symbol] = find_accessor(trailing_expr_raw);

		if (access_symbol.empty())
		{
			return {};
		}

		if (leading_expr_is_command)
		{
			if (access_symbol_local_index != 0)
			{
				return {};
			}
		}

		auto trailing_expr = trailing_expr_raw.substr(access_symbol_local_index + access_symbol.length());

		const auto access_symbol_value_index = static_cast<std::size_t>((trailing_expr_raw.data() + access_symbol_local_index) - value.data());

		const auto leading_expr_length = (access_symbol_value_index - leading_expr_index);

		const auto leading_expr = value.substr(leading_expr_index, leading_expr_length);

		if (truncate_at_first_member)
		{
			auto [next_accessor_index, next_accessor] = find_accessor(trailing_expr);

			if (next_accessor_index != std::string_view::npos)
			{
				trailing_expr = trailing_expr.substr(0, next_accessor_index);
			}
		}

		if (!allow_trailing_command_syntax)
		{
			const auto as_command = parse_command(trailing_expr, true, false);

			const auto& command_name = std::get<0>(as_command);

			if (!command_name.empty())
			{
				return {};
			}
		}

		return { leading_expr, trailing_expr };
	}

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

		if (auto variable_type_begin = remainder.find(':'); variable_type_begin != std::string_view::npos)
		{
			variable_name = util::trim(remainder.substr(0, variable_type_begin));

			if ((variable_type_begin + 1) >= remainder.length())
			{
				return output;
			}

			remainder = util::trim_beginning(remainder.substr(variable_type_begin + 1));

			type_specifier_end = remainder.find_first_of(whitespace_symbols);
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
		}

		if (auto assignment_operator = remainder.find('='); assignment_operator != std::string_view::npos)
		{
			variable_type = util::trim(remainder.substr(0, assignment_operator), true);

			if ((assignment_operator + 1) < remainder.size())
			{
				assignment_expr = util::trim(remainder.substr(assignment_operator + 1));
			}

			return output;
		}
		else
		{
			if (type_specifier_end == std::string_view::npos)
			{
				trailing_expr = remainder;

				return output;
			}

			variable_type = util::trim(remainder.substr(0, type_specifier_end));

			remainder = remainder.substr(type_specifier_end);
		}

		trailing_expr = remainder;

		return output;
	}

	std::size_t find_scope_closing_symbol(std::string_view expr, std::string_view begin_symbol, std::string_view end_symbol, std::size_t position, std::string_view ignore_begin_symbol, std::string_view ignore_end_symbol)
    {
	    if (position == std::string_view::npos)
	    {
		    return std::string_view::npos;
	    }

	    std::size_t begin_symbols_found = 0;

		const bool has_ignore_symbols = (!ignore_begin_symbol.empty() && !ignore_end_symbol.empty());

	    while (position < expr.length())
	    {
			if (has_ignore_symbols)
			{
				const auto ignore_begin_position = expr.find(ignore_begin_symbol, position);

				if (ignore_begin_position != std::string_view::npos)
				{
					const auto ignore_end_position = expr.find(ignore_end_symbol, (ignore_begin_position + 1));

					if (ignore_end_position != std::string_view::npos)
					{
						position = ignore_end_position;

						continue;
					}
				}
			}

		    const auto nearest_scope_begin = expr.find(begin_symbol, position);

		    const auto nearest_scope_end = expr.find(end_symbol, position);

		    if ((nearest_scope_begin != std::string_view::npos) && (nearest_scope_begin < nearest_scope_end))
		    {
			    begin_symbols_found++;

			    position = (nearest_scope_begin + 1);

			    continue;
		    }

		    if (nearest_scope_end == std::string_view::npos)
		    {
			    break;
		    }

		    if (begin_symbols_found > 0)
		    {
			    begin_symbols_found--;

			    position = nearest_scope_end + 1;
		    }
		    else
		    {
			    return nearest_scope_end;
		    }
	    }

	    return std::string_view::npos;
    }

    std::tuple<std::size_t, std::size_t> find_scope_symbols(std::string_view expr, std::string_view begin_symbol, std::string_view end_symbol, std::string_view ignore_begin_symbol, std::string_view ignore_end_symbol)
    {
		std::size_t scope_begin = std::string_view::npos;

		const bool has_ignore_symbols = (!ignore_begin_symbol.empty() && !ignore_end_symbol.empty());

		if (has_ignore_symbols)
		{
			std::size_t position = 0;

			while (position < expr.length())
			{
				scope_begin = expr.find(begin_symbol, position);

				const auto first_ignore_begin_symbol = expr.find(ignore_begin_symbol, position);

				if ((first_ignore_begin_symbol != std::string_view::npos) && (scope_begin > first_ignore_begin_symbol))
				{
					const auto first_ignore_end_symbol = expr.find(ignore_end_symbol, (first_ignore_begin_symbol + 1));

					if ((first_ignore_end_symbol != std::string_view::npos) && (scope_begin < first_ignore_end_symbol))
					{
						position = (first_ignore_end_symbol + 1);
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

			if (position >= expr.length())
			{
				scope_begin = std::string_view::npos;
			}
		}
		else
		{
			scope_begin = expr.find(begin_symbol);
		}

	    if (scope_begin == std::string_view::npos)
	    {
			return { std::string_view::npos, std::string_view::npos };
	    }

	    return
		{
			scope_begin,

			find_scope_closing_symbol
			(
				expr,
				begin_symbol, end_symbol,
				(scope_begin + 1),
				ignore_begin_symbol, ignore_end_symbol
			)
		};
    }

    std::size_t find_closing_parenthesis(std::string_view expr, std::size_t position)
    {
        return find_scope_closing_symbol(expr, "(", ")", position, "\"", "\"");
    }

    std::tuple<std::size_t, std::size_t> find_parentheses(std::string_view expr)
    {
        return find_scope_symbols(expr, "(", ")", "\"", "\"");
    }

	std::size_t find_closing_subscript(std::string_view expr, std::size_t position)
	{
		return find_scope_closing_symbol(expr, "[", "]", position, "\"", "\"");
	}

	std::tuple<std::size_t, std::size_t> find_subscript(std::string_view expr)
	{
		return find_scope_symbols(expr, "[", "]", "\"", "\"");
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

	std::tuple<std::size_t, std::string_view> find_operator(std::string_view expr)
	{
		if (expr.empty())
		{
			return { std::string_view::npos, {} };
		}

		auto expandable_result = [&expr](std::size_t result_index, std::size_t operator_length, auto&&... expansion_symbols) -> std::tuple<std::size_t, std::string_view>
		{
			//assert(result_index < std::numeric_limits<decltype(result_index)>::max());

			// Check for assignment syntax (e.g. `+=`, `-=`):
			if (const auto next_index = (result_index + operator_length); next_index < expr.length())
			{
				if (((expr[next_index] == expansion_symbols) || ...))
				{
					operator_length++;
				}
			}

			return { result_index, expr.substr(result_index, operator_length) };
		};

		if (auto logical_and = expr.find("&&"); logical_and != std::string_view::npos)
		{
			return { logical_and, expr.substr(logical_and, 2) };
		}

		if (auto logical_or = expr.find("||"); logical_or != std::string_view::npos)
		{
			return { logical_or, expr.substr(logical_or, 2) };
		}

		if (auto shift_left = expr.find("<<"); shift_left != std::string_view::npos)
		{
			return expandable_result(shift_left, 2, '=');
		}

		if (auto shift_right = expr.find(">>"); shift_right != std::string_view::npos)
		{
			return expandable_result(shift_right, 2, '=');
		}

		if (auto less_than = expr.find('<'); less_than != std::string_view::npos)
		{
			// Handles `<`, `<=` and `<>` (alternative inequality syntax).
			return expandable_result(less_than, 1, '=', '>');
		}

		if (auto greater_than = expr.find('>'); greater_than != std::string_view::npos)
		{
			return expandable_result(greater_than, 1, '=');
		}

		if (auto single_char_operator = expr.find_first_of("+-~*/%&^|=!"); single_char_operator != std::string_view::npos)
		{
			return expandable_result(single_char_operator, 1, '=');
		}

		return { std::string_view::npos, {} };
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