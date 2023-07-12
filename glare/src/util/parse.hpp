#pragma once

#include "string.hpp"
#include "find.hpp"
#include "type_traits.hpp"

#include <type_traits>
#include <string_view>
#include <string>
#include <tuple>
#include <utility>
#include <array>
//#include <cassert>

namespace util
{
	inline constexpr auto standard_scope_symbols = std::array
	{
		std::pair<std::string_view, std::string_view> { "(", ")" },
		std::pair<std::string_view, std::string_view> { "\"", "\"" },
		std::pair<std::string_view, std::string_view> { "[", "]" },
	};

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

	// Attempts to find the furthest instance of an access symbol. (`::`, `.`, `->`)
	// 
	// If an access symbol could not be found, this will return a tuple with
	// `std::string_view::npos` as the symbol position, and
	// an empty `std::string_view` as the symbol content.
	std::tuple<std::size_t, std::string_view> find_last_accessor(std::string_view expr);

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
	parse_standard_operator_segment
	(
		std::string_view operator_expr,

		bool allow_trailing_expr=true,
		bool beginning_of_string_only=true
	);

	std::tuple
	<
		std::string_view, // leading
		std::string_view, // trailing
		std::size_t       // parsed_length
	>
	parse_member_reference
	(
		const std::string_view value,

		bool allow_command_syntax=false,
		bool match_beginning_only=false,
		bool allow_trailing_command_syntax=true,
		bool truncate_at_first_member=true,
		
		bool truncate_value_at_first_accessor=false,
		bool truncate_command_name_at_first_accessor=false
	);

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
		
		std::string_view type_specification_symbol=":",

		bool allow_trailing_expr=false,
		bool ensure_singular_type_specifier_symbol=true,
		bool unquote_string_literal_as_key_name=true,
		bool allow_key_truncation=false
	);

	// Finds the first `end_symbol` that doesn't already satisfy a `begin_symbol`.
	template <typename IgnoreSymbolArray>
	std::size_t find_scope_closing_symbol(std::string_view expr, std::string_view begin_symbol, std::string_view end_symbol, std::size_t position, const IgnoreSymbolArray& ignore_symbols)
    {
	    if (position == std::string_view::npos)
	    {
		    return std::string_view::npos;
	    }

	    std::size_t begin_symbols_found = 0;

	    while (position < expr.length())
	    {
			const auto nearest_scope_end = find_unescaped(expr, end_symbol, position);

			bool skip_forward = false;

			for (const auto& ignore_entry : ignore_symbols)
			{
				const auto& [ignore_begin_symbol, ignore_end_symbol] = ignore_entry;

				const auto ignore_begin_position = find_unescaped(expr, ignore_begin_symbol, position);

				if ((ignore_begin_position != std::string_view::npos) && ((nearest_scope_end == std::string_view::npos) || (ignore_begin_position < nearest_scope_end)))
				{
					const auto ignore_end_position = find_unescaped(expr, ignore_end_symbol, (ignore_begin_position + 1));

					if (ignore_end_position != std::string_view::npos)
					{
						position = ignore_end_position;

						skip_forward = true;

						break;
					}
				}
			}

			if (skip_forward)
			{
				continue;
			}

		    const auto nearest_scope_begin = find_unescaped(expr, begin_symbol, position);

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

	inline std::size_t find_scope_closing_symbol
	(
		std::string_view expr,
		std::string_view begin_symbol, std::string_view end_symbol,
		std::size_t position,
		std::string_view ignore_begin_symbol, std::string_view ignore_end_symbol
	)
	{
		return find_scope_closing_symbol
		(
			expr,
			begin_symbol, end_symbol,
			position,

			std::array
			{
				std::pair { ignore_begin_symbol, ignore_end_symbol }
			}
		);
	}

	inline std::size_t find_scope_closing_symbol
	(
		std::string_view expr,
		std::string_view begin_symbol, std::string_view end_symbol,
		std::size_t position
	)
	{
		return find_scope_closing_symbol
		(
			expr,
			begin_symbol, end_symbol,
			position,

			std::array<std::pair<std::string_view, std::string_view>, 0> {}
		);
	}

	// Searches `expr` for `begin_symbol`, skipping scopes enclosed by `ignore_symbols`.
	template <typename IgnoreSymbolArray>
	std::size_t find_scope_beginning_symbol
	(
		std::string_view expr,
		std::string_view begin_symbol,
		const IgnoreSymbolArray& ignore_symbols
	)
	{
		std::size_t scope_begin = std::string_view::npos;

		std::size_t position = 0;

		while (position < expr.length())
		{
			scope_begin = find_unescaped(expr, begin_symbol, position);

			bool exit_scope_search = true;

			for (const auto& ignore_pair : ignore_symbols)
			{
				const auto& [ignore_begin_symbol, ignore_end_symbol] = ignore_pair;

				const auto first_ignore_begin_symbol = find_unescaped(expr, ignore_begin_symbol, position);

				if ((first_ignore_begin_symbol != std::string_view::npos) && (scope_begin > first_ignore_begin_symbol))
				{
					const auto first_ignore_end_symbol = find_unescaped(expr, ignore_end_symbol, (first_ignore_begin_symbol + 1));

					if ((first_ignore_end_symbol != std::string_view::npos) && (scope_begin < first_ignore_end_symbol))
					{
						position = (first_ignore_end_symbol + 1);

						exit_scope_search = false;

						break;
					}
				}
			}

			if (exit_scope_search)
			{
				break;
			}
		}

		if (position >= expr.length())
		{
			return std::string_view::npos;
		}

		return scope_begin;
	}

	// Finds the leading `begin_symbol` and trailing `end_symbol` of an expression.
	template <typename IgnoreSymbolArray>
	std::tuple<std::size_t, std::size_t> find_scope_symbols
	(
		std::string_view expr,
		std::string_view begin_symbol, std::string_view end_symbol,
		const IgnoreSymbolArray& ignore_symbols
	)
    {
		const auto scope_begin = find_scope_beginning_symbol(expr, begin_symbol, ignore_symbols);

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
				(scope_begin + begin_symbol.length()),
				ignore_symbols
			)
		};
    }

	inline std::tuple<std::size_t, std::size_t> find_scope_symbols
	(
		std::string_view expr,
		std::string_view begin_symbol, std::string_view end_symbol,
		std::string_view ignore_begin_symbol, std::string_view ignore_end_symbol
	)
	{
		return find_scope_symbols
		(
			expr, begin_symbol, end_symbol,

			std::array
			{
				std::pair { ignore_begin_symbol, ignore_end_symbol }
			}
		);
	}

	inline std::tuple<std::size_t, std::size_t> find_scope_symbols
	(
		std::string_view expr,
		std::string_view begin_symbol, std::string_view end_symbol
	)
	{
		return find_scope_symbols
		(
			expr, begin_symbol, end_symbol,

			std::array<std::pair<std::string_view, std::string_view>, 0> {}
		);
	}

	// Copies N-1 symbol pairs from `symbols_in` to `symbols_out`, where
	// the removed element is the object pointed to by `pair_to_ignore`.
	template
	<
		typename SymbolPairArray,
		typename SymbolPairType = typename SymbolPairArray::value_type // std::pair<std::string_view, std::string_view>
	>
	void ignore_symbol_pair(const SymbolPairArray& symbols_in, const SymbolPairType* pair_to_ignore, auto& symbols_out)
	{
		/*
		// Alternative implementation:
		const auto entry_index = ...;

		for (std::size_t ignored_index = 0; ignored_index < symbols_in.size(); ignored_index)
		{
			if (ignored_index < entry_index)
			{
				symbols_out[ignored_index] = symbols_in[ignored_index];
			}
			else if (ignored_index == entry_index)
			{
				continue;
			}
			else
			{
				// We subtract one from `ignored_index` to adjust for `entry_index` not being included.
				const auto translated_index = (ignored_index - 1);

				symbols_out[translated_index] = symbols_in[ignored_index];
			}
		}
		*/

		// STL algorithm implementation:
		std::copy_if
		(
			symbols_in.begin(), symbols_in.end(), symbols_out.begin(),
			
			[pair_to_ignore](const auto& pair_to_copy)
			{
				return ((&pair_to_copy) != pair_to_ignore);
			}
		);
	}

	// Creates an array of N-1 symbol pairs from `symbols`, where the
	// removed element is the object pointed to by `pair_to_ignore`.
	template
	<
		typename SymbolPairArray,
		typename SymbolPairType = typename SymbolPairArray::value_type // std::pair<std::string_view, std::string_view>
	>
	auto ignore_symbol_pair(const SymbolPairArray& symbols, const SymbolPairType* pair_to_ignore)
	{
		using IgnoredArrayType = std::array
		<
			SymbolPairType,
			(array_size<SymbolPairArray> - 1)
		>;

		IgnoredArrayType ignored_symbols;

		ignore_symbol_pair(symbols, pair_to_ignore, ignored_symbols);

		return ignored_symbols;
	}

	template
	<
		typename ScopeSymbolArray,
		typename SymbolSearchFn,
		typename SymbolPositionCompareFn,
		typename SymbolPairType = typename ScopeSymbolArray::value_type, // std::pair<std::string_view, std::string_view>
		typename IgnoredSymbolArray = std::array<SymbolPairType, (array_size<ScopeSymbolArray> - 1)>
	>
	std::tuple<std::size_t, const SymbolPairType*, IgnoredSymbolArray> // auto
	find_scope_symbol_ex
	(
		std::string_view expr, const ScopeSymbolArray& scope_symbols,
		SymbolSearchFn&& search_for_symbol, SymbolPositionCompareFn&& compare_symbol_position
	)
	{
		auto earliest_scope_found = std::tuple<std::size_t, const SymbolPairType*, IgnoredSymbolArray> { std::string_view::npos, {}, {}};

		for (const auto& scope_entry : scope_symbols)
		{
			// Build an ignored-scopes array containing all other symbol pairs.
			// TODO: Look into better way of handling this. (i.e. without creating an array)
			auto ignored_scope_symbols = ignore_symbol_pair(scope_symbols, &scope_entry);

			if (const auto scope_begin = search_for_symbol(expr, scope_entry, ignored_scope_symbols); scope_begin != std::string_view::npos)
			{
				auto& earliest_scope_index = std::get<0>(earliest_scope_found);

				if (compare_symbol_position(scope_begin, earliest_scope_index))
				{
					auto& earliest_scope_pair = std::get<1>(earliest_scope_found);
					auto& earliest_scope_ignored_symbols = std::get<2>(earliest_scope_found);

					earliest_scope_index = scope_begin;
					earliest_scope_pair = &scope_entry;
					earliest_scope_ignored_symbols = std::move(ignored_scope_symbols);
				}
			}
		}

		return earliest_scope_found;
	}

	template <typename ScopeSymbolArray>
	auto find_first_scope_beginning_symbol(std::string_view expr, const ScopeSymbolArray& scope_symbols)
	{
		return find_scope_symbol_ex
		(
			expr, scope_symbols,

			[](const auto& expr, const auto& scope_entry, const auto& ignore_symbols)
			{
				// Retrieve the beginning symbol.
				const auto& begin_symbol = std::get<0>(scope_entry);

				// Look for the scope's beginning.
				return find_scope_beginning_symbol(expr, begin_symbol, ignore_symbols);
			},

			[](auto scope_begin, auto earliest_scope_index)
			{
				// Always choose the earliest symbol.
				return (scope_begin < earliest_scope_index);
			}
		);
	}

	template <typename ScopeSymbolArray>
	auto find_last_scope_beginning_symbol(std::string_view expr, const ScopeSymbolArray& scope_symbols)
	{
		return find_scope_symbol_ex
		(
			expr, scope_symbols,

			[](const auto& expr, const auto& scope_entry, const auto& ignore_symbols)
			{
				// Retrieve the beginning symbol.
				const auto& begin_symbol = std::get<0>(scope_entry);

				// Look for the scope's beginning.
				return find_scope_beginning_symbol(expr, begin_symbol, ignore_symbols);
			},

			[](auto scope_begin, auto furthest_scope_index)
			{
				// Always choose the furthest symbol.
				return (scope_begin >= furthest_scope_index);
			}
		);
	}

	template <typename ScopeSymbolArray, typename Callback>
	void enumerate_unscoped_substrings
	(
		std::string_view expr,
		const ScopeSymbolArray& scope_symbols,
		Callback&& callback,
		std::size_t offset=0
	)
	{
		std::size_t position = offset;

		while (position < expr.length())
		{
			const auto remainder = expr.substr(position);

			const auto [scope_begin_index, scope_pair_ptr, scope_ignored_symbols] = find_first_scope_beginning_symbol(remainder, scope_symbols);

			if (scope_begin_index == std::string_view::npos)
			{
				callback(remainder);

				break;
			}
			else
			{
				const auto& scope_begin_symbol = std::get<0>(*scope_pair_ptr);
				const auto& scope_end_symbol = std::get<1>(*scope_pair_ptr);

				const auto scope_area_index = (scope_begin_index + scope_begin_symbol.length());

				if (scope_area_index < remainder.length())
				{
					const auto scope_area = remainder.substr(scope_area_index);

					constexpr std::size_t scope_area_search_offset = 0;

					const auto relative_scope_end_index = find_scope_closing_symbol
					(
						scope_area, scope_begin_symbol, scope_end_symbol,
						scope_area_search_offset, scope_ignored_symbols
					);

					if (relative_scope_end_index == std::string_view::npos)
					{
						if constexpr (array_size<ScopeSymbolArray> > 1)
						{
							// Scope mismatch; try this substring again with the current scope-pair removed.
							// (i.e. use our ignored symbols list as the list of possible symbols)
							enumerate_unscoped_substrings(remainder, scope_ignored_symbols, callback);
						}
						else
						{
							callback(remainder);
						}

						break;
					}
					else
					{
						// Convert the scope's ending-index from `scope_area`-relative to `expr`-relative.
						const auto scope_end_position = static_cast<std::size_t>((scope_area.data() + relative_scope_end_index) - expr.data());

						const auto& preceding_content_length = scope_begin_index;

						if (preceding_content_length > 0)
						{
							// Report to the caller about any content found before this scope began:
							const auto unscoped_preceding = remainder.substr(0, scope_begin_index);

							if constexpr (std::is_invocable_r_v<bool, Callback, decltype(unscoped_preceding)>)
							{
								if (!callback(unscoped_preceding))
								{
									break;
								}
							}
							else
							{
								callback(unscoped_preceding);
							}
						}

						// Seek past this scope, then continue processing.
						position = (scope_end_position + scope_end_symbol.length());
					}
				}
				else
				{
					// This 'scope' has no end and no content.
					// Report the remainder to the user, then exit.
					callback(remainder);

					break;
				}
			}
		}
	}

	// Searches substrings of `str` for `target_symbol` using `find_fn`, starting at `offset`.
	// Scopes enclosed by any of the `std::string_view` pairs found in `scope_symbols` are ignored.
	template <typename ScopeSymbolArray, typename FindFn>
	std::size_t find_unscoped(std::string_view str, std::string_view target_symbol, std::size_t offset, const ScopeSymbolArray& scope_symbols, FindFn&& find_fn)
	{
		std::size_t result = std::string_view::npos;

		enumerate_unscoped_substrings
		(
			str, scope_symbols,

			[&str, &target_symbol, &find_fn, &result](const auto& unscoped_substr)
			{
				// Bounds-check for `unscoped_substr`.
				//assert((unscoped_substr.data() >= str.data()) && ((unscoped_substr.data() + unscoped_substr.length()) <= (str.data() + str.length())));

				if (auto substr_result = find_fn(unscoped_substr, target_symbol); substr_result != std::string_view::npos)
				{
					// Translate from `unscoped_substr`'s index-space to `str`'s index-space.
					result = static_cast<std::size_t>((unscoped_substr.data() + substr_result) - str.data());

					// Result found, stop searching.
					return false;
				}

				// Continue searching for `target_symbol`.
				return true;
			},

			offset
		);

		return result;
	}

	/*
		Searches `str` for `target_symbol`, starting at `offset`.
		Scopes enclosed by any of the `std::string_view` pairs found in `scope_symbols` are ignored.

		This overload currently uses `std::string_view::find` as its default search routine.
	*/
	template <typename ScopeSymbolArray>
	std::size_t find_unscoped(std::string_view str, std::string_view target_symbol, std::size_t offset, const ScopeSymbolArray& scope_symbols)
	{
		return find_unscoped
		(
			str, target_symbol, offset, scope_symbols,

			[](const auto& unscoped_substr, const auto& target_symbol)
			{
				return unscoped_substr.find(target_symbol);
			}
		);
	}

	/*
		Searches `str` for `target_symbol`, starting at `offset`.

		This overload uses a pre-defined set of scope symbols to ignore. (`standard_scope_symbols`)
		This overload currently uses `std::string_view::find` as its default search routine.
	*/
	inline std::size_t find_unscoped(std::string_view str, std::string_view target_symbol, std::size_t offset=0)
	{
		return find_unscoped
		(
			str, target_symbol, offset,
			standard_scope_symbols
		);
	}

	// Searches for the earliest instance of a `target_symbols` entry in `expr`, where
	// the entry in question is not within a scope found in the `scope_symbols` container.
	template <typename ScopeSymbols, typename ...TargetSymbols>
	std::tuple<std::size_t, std::string_view> find_first_of_unscoped
	(
		std::string_view expr, const ScopeSymbols& scope_symbols,
		TargetSymbols&&... target_symbols
	)
	{
		auto result_symbol = std::string_view {};

		// NOTE: Unlike `result_symbol`, `result_index` needs to be translated to be `expr`-relative by `find_unscoped`.
		// For this reason, we use the return value, rather than simply handling `result_index` as an out-parameter.
		auto result_index = find_unscoped
		(
			expr, "", 0, scope_symbols,

			[&](const auto& substr, const auto& placeholder)
			{
				auto [sub_result_index, sub_result_symbol] = find_first_of_ex
				(
					substr,
					std::string_view::npos,
					std::forward<TargetSymbols>(target_symbols)...
				);

				if (sub_result_index != std::string_view::npos)
				{
					// Store a view to the symbol we found.
					result_symbol = sub_result_symbol;
				}

				return sub_result_index;
			}
		);

		return { result_index, result_symbol };
	}

	// Finds the first closing parenthesis (')') that doesn't already satisfy an enclosed expression.
	std::size_t find_closing_parenthesis(std::string_view expr, std::size_t position=0);

	// Finds the leading ('(') and trailing (')') parentheses in an expression.
    std::tuple<std::size_t, std::size_t> find_parentheses(std::string_view expr);

	// Finds the first closing bracket (']') that doesn't already satisfy an enclosed expression.
	std::size_t find_closing_subscript(std::string_view expr, std::size_t position=0);

	// Finds the leading ('[') and trailing (']') subscript brackets in an expression.
	std::tuple<std::size_t, std::size_t> find_subscript(std::string_view expr);

	std::tuple<std::size_t, std::string_view>
	find_assignment_operator(std::string_view expr, bool check_compound_operators=true, bool validate_scope_bounds=true, bool disallow_right_of_scope=false);

	// Attempts to find the first instance of a logical operator (`&&`, `||`, [`^`]) in `expr`.
	std::tuple<std::size_t, std::string_view> find_logic_operator(std::string_view expr, bool include_xor=false);

	// Attempts to find the first instance of an operator in `expr`. (Excludes accessors; e.g. `::`, `.`)
	std::tuple<std::size_t, std::string_view> find_operator(std::string_view expr);

	std::tuple<std::size_t, std::size_t>
	find_quotes(std::string_view str, std::string_view begin_quote_symbol="\"", std::string_view end_quote_symbol="\"");
}