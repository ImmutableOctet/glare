#include "parse.hpp"

#include <engine/meta/hash.hpp>

#include <util/string.hpp>
#include <util/parse.hpp>
#include <util/json.hpp>

#include <regex>

namespace engine
{
	std::optional<Timer::Duration> parse_time_duration(const std::string& time_expr) // std::string_view
	{
		using namespace engine::literals;

		const auto time_rgx = std::regex("([\\d\\.]+)\\s*(.*)"); // (d|h|m|s|ms|us)?

		if (std::smatch rgx_match; std::regex_search(time_expr.begin(), time_expr.end(), rgx_match, time_rgx))
		{
			const auto numeric_str = util::match_view(time_expr, rgx_match, 1);

			const auto time_symbol = util::match_view(time_expr, rgx_match, 2);

			if (const auto number = util::from_string<Timer::DurationRaw>(numeric_str))
			{
				return Timer::to_duration(*number, time_symbol);
			}

			if (const auto floating_point = util::from_string<float>(numeric_str)) // double
			{
				return Timer::to_duration(*floating_point, time_symbol);
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
		std::size_t initial_offset,
		const MetaParsingInstructions* opt_parsing_instructions,
		bool disallow_command_as_symbol
	)
	{
		std::size_t offset = 0;

		auto remainder = std::string_view {}; // content;

		auto seek_forward = [&content, &offset, &remainder](std::size_t characters_processed) -> std::string_view
		{
			offset += characters_processed;
			remainder = content.substr(offset);

			return remainder;
		};

		seek_forward(initial_offset);

		auto get_access_operator = [&remainder, &seek_forward]() -> std::string_view
		{
			// Search for access operator:
			const auto [access_symbol_position, access_symbol] = util::find_accessor(remainder);

			if (access_symbol_position == std::string_view::npos)
			{
				return {};
			}

			// Ensure there wasn't a standard (i.e. non-accessor) operator
			// between the beginning of the remainder and the access operator:
			const auto possible_operator_cutoff_segment = remainder.substr(0, access_symbol_position);

			if (std::get<0>(util::find_operator(possible_operator_cutoff_segment)) != std::string_view::npos)
			{
				return {};
			}

			assert(!access_symbol.empty());

			// Seek past the accessor we found.
			seek_forward(access_symbol_position + access_symbol.length());

			return access_symbol;
		};

		auto entity_target_parse_result = EntityTarget::parse_type(remainder, opt_parsing_instructions);
		
		if (entity_target_parse_result)
		{
			const auto entity_ref_length = std::get<1>(*entity_target_parse_result);

			seek_forward(entity_ref_length);

			get_access_operator();
		}

		auto get_symbol = [disallow_command_as_symbol, &remainder, &seek_forward]
		(bool allow_operator_symbol_in_command_name=false, bool truncate_value_at_operator_symbol=true) -> std::tuple<std::string_view, bool>
		{
			const auto
			[
				symbol_identifier,
				symbol_content,
				
				symbol_trailing_expr,
				
				symbol_is_string_content,
				symbol_is_command,

				symbol_parsed_length
			] = util::parse_command_or_value
			(
				remainder,
				true, true, true, true,
				allow_operator_symbol_in_command_name,
				truncate_value_at_operator_symbol
			);

			if (symbol_identifier.empty())
			{
				return {};
			}

			auto symbol = std::string_view {};

			if (symbol_is_command)
			{
				if (disallow_command_as_symbol)
				{
					return {};
				}

				symbol = remainder.substr(0, symbol_parsed_length);
				seek_forward(symbol_parsed_length);
			}
			else
			{
				symbol = symbol_content; // symbol_identifier;

				const auto symbol_content_end_point = (symbol_trailing_expr.empty())
					? static_cast<std::size_t>((symbol_content.data() + symbol_content.length()) - remainder.data())
					: static_cast<std::size_t>(symbol_trailing_expr.data() - remainder.data())
				;

				seek_forward(symbol_content_end_point);
			}

			return { symbol, symbol_is_command };
		};

		const auto [first_symbol, first_symbol_is_command] = get_symbol();

		if (first_symbol.empty())
		{
			return {};
		}

		auto access_operator = get_access_operator();

		auto second_symbol = std::string_view {};
		auto second_symbol_is_command = false;

		if (!access_operator.empty())
		{
			const auto second_symbol_result = get_symbol();

			second_symbol = std::get<0>(second_symbol_result);
			second_symbol_is_command = std::get<1>(second_symbol_result);
		}

		return { entity_target_parse_result, first_symbol, second_symbol, access_operator, first_symbol_is_command, second_symbol_is_command, offset };
	}

	std::tuple
	<
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
		std::size_t initial_offset,
		std::string_view allowed_operators,
		
		bool allow_missing_operator,
		bool allow_empty_trailing_value,
		bool allow_scope_as_implied_operator,
		bool truncate_at_logical_operators,
		bool disallow_command_as_symbol,

		const MetaParsingInstructions* opt_parsing_instructions
	)
	{
		auto content = condition_or_assignment.substr(initial_offset);

		std::size_t latest_scope_end = std::string_view::npos;
		
		while (!content.empty())
		{
			auto [scope_begin, scope_end] = util::find_parentheses(content);

			if ((scope_begin == 0) && (scope_end != std::string_view::npos))
			{
				// NOTE: +1/-1 to account for scope symbols. -- '(', ')'
				content = content.substr((scope_begin + 1), (scope_end-1));
				latest_scope_end = scope_end;
			}
			else
			{
				break;
			}
		}

		if (content.empty())
		{
			return {};
		}

		const bool is_enclosed_in_scope = (latest_scope_end != std::string_view::npos);

		std::size_t offset = 0;

		if ((!is_enclosed_in_scope) && (truncate_at_logical_operators))
		{
			const auto first_non_whitespace = content.find_first_not_of(util::whitespace_symbols);

			if (first_non_whitespace == std::string_view::npos)
			{
				return {};
			}

			const auto [leading_logic_operator_position, leading_logic_operator_symbol] = util::find_logic_operator(content, false);

			if (leading_logic_operator_position == first_non_whitespace)
			{
				offset = (leading_logic_operator_position + leading_logic_operator_symbol.length());
			}
		}

		auto
		[
			entity_target_parse_result,
			first_symbol, second_symbol,
			access_operator,
			first_symbol_is_command,
			second_symbol_is_command,
			qualified_reference_offset
		] = parse_qualified_reference(content, offset, opt_parsing_instructions, false);

		if (disallow_command_as_symbol)
		{
			if (((!first_symbol.empty()) && first_symbol_is_command) || ((!second_symbol.empty()) && second_symbol_is_command))
			{
				return {};
			}
		}

		offset = qualified_reference_offset;

		auto entity_ref = std::string_view {};

		if (entity_target_parse_result)
		{
			const auto entity_ref_length = std::get<1>(*entity_target_parse_result);

			entity_ref = content.substr(0, entity_ref_length);
		}

		auto remainder = content.substr(offset);

		auto seek_forward = [&content, &offset, &remainder](std::size_t characters_processed) -> std::string_view
		{
			offset += characters_processed;
			remainder = content.substr(offset);

			return remainder;
		};

		const auto [operator_position, operator_symbol] = util::find_operator(remainder);

		if (operator_position == std::string_view::npos)
		{
			if ((!allow_missing_operator) && (!is_enclosed_in_scope || !allow_scope_as_implied_operator))
			{
				return {};
			}
		}
		else
		{
			assert(!operator_symbol.empty());
			
			if (!allowed_operators.empty() && !allowed_operators.contains(operator_symbol))
			{
				return {};
			}

			seek_forward(operator_position + operator_symbol.length());
		}

		//auto compared_or_assigned_value = get_symbol(true, false);
		auto compared_or_assigned_value = util::trim(remainder);

		std::size_t updated_offset = 0;
		
		if (compared_or_assigned_value.empty() && access_operator.empty())
		{
			// This handles an edge-case where there are no required operators,
			// but the second value supplied is intended as the (compared or assigned) trailing value.
			if ((operator_position == std::string_view::npos) && !second_symbol.empty())
			{
				compared_or_assigned_value = std::move(second_symbol);
				second_symbol = {};
			}
		}

		if (compared_or_assigned_value.empty())
		{
			if ((!allow_empty_trailing_value) && (!is_enclosed_in_scope || !allow_scope_as_implied_operator))
			{
				return {};
			}

			if (latest_scope_end == std::string_view::npos)
			{
				const auto entry_point_offset = static_cast<std::size_t>(content.data() - condition_or_assignment.data());

				updated_offset = (entry_point_offset + offset);
			}
			else
			{
				// NOTE: + 1 to account for ')' symbol.
				updated_offset = (latest_scope_end + 1);
			}
		}
		else
		{
			if (truncate_at_logical_operators)
			{
				const auto [logic_operator_index, logic_operator] = util::find_logic_operator(compared_or_assigned_value, false);

				if (logic_operator_index != std::string_view::npos)
				{
					compared_or_assigned_value = util::trim(compared_or_assigned_value.substr(0, logic_operator_index));
				}
			}

			if (latest_scope_end == std::string_view::npos)
			{
				updated_offset = static_cast<std::size_t>((compared_or_assigned_value.data() + compared_or_assigned_value.length()) - condition_or_assignment.data());
			}
			else
			{
				// NOTE: + 1 to account for ')' symbol.
				updated_offset = (latest_scope_end + 1);
			}
		}

		return { entity_ref, first_symbol, second_symbol, operator_symbol, compared_or_assigned_value, updated_offset };
	}
}