#pragma once

#include "type_traits.hpp"

//#include <iterator>
#include <algorithm>
#include <utility>
#include <type_traits>
#include <string>
#include <string_view>
#include <optional>
#include <tuple>

namespace util
{
    namespace impl
    {
        template <typename Key>
        inline constexpr bool key_has_value_semantics_v = (std::is_copy_constructible_v<std::decay_t<Key>> || std::is_move_constructible_v<std::decay_t<Key>>);

        template <typename Key>
        inline constexpr bool is_regular_key_v = (std::is_default_constructible_v<std::decay_t<Key>> && key_has_value_semantics_v<Key>);

        template <typename Key>
        inline constexpr bool key_is_optional_compatible_v = ((!is_regular_key_v<Key>) && (key_has_value_semantics_v<Key>) && is_optional_compatible_v<std::decay_t<Key>>);

        template <typename Key>
        inline constexpr bool find_returns_tuple_v =
		(
			is_regular_key_v<Key> // std::decay_t<Key>
			||
			(is_c_str_v<Key> || is_specialization_v<std::decay_t<Key>, std::basic_string> || is_specialization_v<std::decay_t<Key>, std::basic_string_view>)
			||
			(key_has_value_semantics_v<Key> && key_is_optional_compatible_v<Key>)
		);

        // Slightly faster implementation for single return-value on string.
        template <typename ContainerType, typename ...Keys>
        std::size_t find_first_of_string_impl(const ContainerType& input, Keys&&... keys)
        {
            constexpr bool is_string = (std::is_same_v<std::string, std::decay_t<ContainerType>>);
            constexpr bool is_string_view = (std::is_same_v<std::string_view, std::decay_t<ContainerType>>);

            // TODO: Add slightly more generic check for `npos`.
            if constexpr (is_string || is_string_view)
            {
                static_assert(ContainerType::npos == static_cast<std::size_t>(-1));
            }

            return min(input.find(keys)...);
        }
    }

	template <typename Key>
	inline constexpr bool find_returns_tuple_v = util::impl::find_returns_tuple_v<Key>;

	/*
		Attempts to finds `key` in `input`.
		The natural return type of this function is a tuple containing the find result <0> and the `key` specified <1>.

		If the find-result is equal to `end_value`, an empty key will be used. (see below for details)

		If `Key` is default constructible, and copy/move constructible, the
		`key` specified will be forwarded, assuming a non-`end_value` result.

		If `Key` is not default constructible, but is copy or move constructible,
		an `std::optional<Key>` will be used in `Key`'s place <1>.
		
		If `Key` does not meet any of the above requirements, the return-value of this
		function is the result of `ContainerType::find`/`std::find`. (Usually an iterator)

		See also: `find_returns_tuple_v`
	*/
	template <typename ContainerType, typename EndType, typename Key, typename ...FindArgs>
    auto find(const ContainerType& input, EndType&& end_value, Key&& key, FindArgs&&... find_args)
        //-> std::tuple<decltype(input.find(key)), Key>
    {
        using namespace util::impl;

        auto it = input.find(key, std::forward<FindArgs>(find_args)...);

        if (it != end_value)
        {
            if constexpr (is_regular_key_v<Key>)
            {
                return std::tuple { std::move(it), std::forward<Key>(key) };
            }
            else if constexpr (is_c_str_v<Key> || is_specialization_v<std::decay_t<Key>, std::basic_string> || is_specialization_v<std::decay_t<Key>, std::basic_string_view>)
            {
                return std::tuple { std::move(it), key }; // std::forward<std::decay_t<Key>>(key)
            }
            else if constexpr (key_has_value_semantics_v<Key> && key_is_optional_compatible_v<Key>)
            {
                return std::tuple { std::move(it), std::optional<Key>(std::forward<Key>(key)) };
            }
        }

        if constexpr (is_regular_key_v<Key>)
        {
            return std::tuple { std::move(it), Key {} };
        }
        else if constexpr (is_c_str_v<Key> || is_specialization_v<std::decay_t<Key>, std::basic_string> || is_specialization_v<std::decay_t<Key>, std::basic_string_view>)
        {
            return std::tuple { std::move(it), std::decay_t<Key> {} };
        }
        else if constexpr (key_has_value_semantics_v<Key> && key_is_optional_compatible_v<Key>)
        {
            return std::tuple { std::move(it), std::optional<Key>(std::nullopt) };
        }
        else
        {
            return it;
        }
    }

	template <typename ContainerType, typename EndType, typename Key>
	auto find_any_ex(const ContainerType& input, EndType&& end_value, Key&& key)
	{
		return find(input, std::forward<EndType>(end_value), std::forward<Key>(key));
	}

	// Returns an iterator to an entry found in `input` matching one of the keys specified.
	template <typename ContainerType, typename EndType, typename Key, typename ...Keys>
	auto find_any_ex(const ContainerType& input, EndType&& end_value, Key&& key, Keys&&... keys)
	{
		constexpr bool key_has_value_semantics = (std::is_copy_constructible_v<Key> || std::is_move_constructible_v<Key>);
		constexpr bool is_regular_key = (std::is_default_constructible_v<Key> && key_has_value_semantics);

		auto result = find_any_ex(input, end_value, std::forward<Key>(key));

		if constexpr (key_has_value_semantics || is_regular_key)
		{
			if (std::get<0>(result) != end_value)
			{
				return result;
			}
		}
		else
		{
			if (result != end_value)
			{
				return result;
			}
		}

		return find_any_ex(input, std::forward<EndType>(end_value), std::forward<Keys>(keys)...);
	}

	// Returns an iterator to an entry found in `input` matching one of the keys specified.
	template <typename ContainerType, typename Key>
	auto find_any(const ContainerType& input, Key&& key)
	{
		return input.find(key);
	}

	// Returns an iterator to an entry found in `input` matching one of the keys specified.
	template <typename ContainerType, typename Key, typename ...Keys>
	auto find_any(const ContainerType& input, Key&& key, Keys&&... keys)
	{
		auto result = find_any_ex(input, input.end(), std::forward<Key>(key), std::forward<Keys>(keys)...);

		return std::get<0>(result);
	}

	// Returns the first valid find result in `input` using the keys specified.
	// NOTE: Searching is performed in-order until a result is found.
	template <typename Key, typename ...Keys>
	auto find_any(const std::string& input, Key&& key, Keys&&... keys)
	{
		auto result = find_any_ex(input, std::string::npos, std::forward<Key>(key), std::forward<Keys>(keys)...);

		return std::get<0>(result);
	}

	// Returns the first valid find result in `input` using the keys specified.
	// NOTE: Searching is performed in-order until a result is found.
	template <typename Key, typename ...Keys>
	auto find_any(const std::string_view& input, Key&& key, Keys&&... keys)
	{
		auto result = find_any_ex(input, std::string_view::npos, std::forward<Key>(key), std::forward<Keys>(keys)...);

		return std::get<0>(result);
	}

	template <typename ContainerType, typename EndType, typename Key>
    auto find_first_of_ex(const ContainerType& input, EndType&& end_value, Key&& key)
    {
        return find(input, std::forward<EndType>(end_value), std::forward<Key>(key));
    }

    template <typename ContainerType, typename EndType, typename Key, typename ...Keys>
    auto find_first_of_ex(const ContainerType& input, EndType&& end_value, Key&& key, Keys&&... keys)
    {
        auto current = find_first_of_ex(input, end_value, std::forward<Key>(key));

        if constexpr (sizeof...(Keys) == 0)
        {
            return current;
        }
        else
        {
            constexpr bool is_tuple_return_value = find_returns_tuple_v<Key>; // !std::is_same_v<std::decay_t<EndType>, std::decay_t<decltype(current)>>;

            bool current_is_valid = false;

            if constexpr (is_tuple_return_value)
            {
                current_is_valid = (std::get<0>(current) != end_value);
            }
            else
            {
                current_is_valid = (current != end_value);
            }

            auto next = find_first_of_ex(input, std::forward<EndType>(end_value), std::forward<Keys>(keys)...);

            if (current_is_valid)
            {
                if constexpr (is_tuple_return_value)
                {
                    if (std::get<0>(current) <= std::get<0>(next))
                    {
                        return current;
                    }
                }
                else
                {
                    if (current <= next)
                    {
                        return current;
                    }
                }
            }
        
            return next;
        }
    }

    template <typename ContainerType, typename ...Keys>
    auto find_first_of(const ContainerType& input, Keys&&... keys)
    {
        auto result = find_first_of_ex(input, input.end(), std::forward<Keys>(keys)...);

        return std::get<0>(result);
    }

    template <typename ...Keys>
    std::size_t find_first_of(const std::string& input, Keys&&... keys)
    {
        //auto result = find_first_of_ex(input, std::string::npos, std::forward<Keys>(keys)...);
        //return std::get<0>(result);

        return util::impl::find_first_of_string_impl(input, std::forward<Keys>(keys)...);
    }

    template <typename ...Keys>
    std::size_t find_first_of(const std::string_view& input, Keys&&... keys)
    {
        //auto result = find_first_of_ex(input, std::string_view::npos, std::forward<Keys>(keys)...);
        //return std::get<0>(result);

        return util::impl::find_first_of_string_impl(input, std::forward<Keys>(keys)...);
    }

    template <typename ContainerType, typename EndType, typename Key>
    auto find_last_of_ex(const ContainerType& input, EndType&& end_value, Key&& key)
    {
		constexpr bool is_tuple_return_value = find_returns_tuple_v<Key>; // !std::is_same_v<std::decay_t<EndType>, std::decay_t<decltype(current)>>;

		using position_t = std::conditional_t
		<
			is_tuple_return_value,
			decltype(std::get<0>(find(input, end_value, key))), // std::void_t<...>
			decltype(find(input, end_value, key))
		>;

		auto result = find(input, end_value, key);

		if constexpr (is_tuple_return_value)
		{
			while (true)
			{
				auto next_result = find(input, end_value, key, (std::get<0>(result) + 1));

				if (std::get<0>(next_result) == end_value)
				{
					break;
				}
				else
				{
					result = std::move(next_result);
				}
			}
		}
		else
		{
			while (true)
			{
				auto next_result = find(input, end_value, key, (result + 1));

				if (next_result == end_value)
				{
					break;
				}
				else
				{
					result = std::move(next_result);
				}
			}
		}

		return result;
    }

    template <typename ContainerType, typename EndType, typename Key, typename ...Keys>
    auto find_last_of_ex(const ContainerType& input, EndType&& end_value, Key&& key, Keys&&... keys)
    {
        auto current = find_last_of_ex(input, end_value, std::forward<Key>(key));

        if constexpr (sizeof...(Keys) == 0)
        {
            return current;
        }
        else
        {
            constexpr bool is_tuple_return_value = find_returns_tuple_v<Key>; // !std::is_same_v<std::decay_t<EndType>, std::decay_t<decltype(current)>>;

            bool current_is_valid = false;

            if constexpr (is_tuple_return_value)
            {
                current_is_valid = (std::get<0>(current) != end_value);
            }
            else
            {
                current_is_valid = (current != end_value);
            }

            auto next = find_last_of_ex(input, std::forward<EndType>(end_value), std::forward<Keys>(keys)...);

            if (current_is_valid)
            {
                if constexpr (is_tuple_return_value)
                {
                    if (std::get<0>(current) > std::get<0>(next))
                    {
                        return current;
                    }
                }
                else
                {
                    if (current > next)
                    {
                        return current;
                    }
                }
            }

            return next;
        }
    }

    template <typename ContainerType, typename ...Keys>
    auto find_last_of(const ContainerType& input, Keys&&... keys)
    {
        auto result = find_last_of_ex(input, input.end(), std::forward<Keys>(keys)...);

        return std::get<0>(result);
    }

    template <typename ...Keys>
    std::size_t find_last_of(const std::string& input, Keys&&... keys)
    {
        auto result = find_last_of_ex(input, std::string::npos, std::forward<Keys>(keys)...);

        return std::get<0>(result);
    }

    template <typename ...Keys>
    std::size_t find_last_of(const std::string_view& input, Keys&&... keys)
    {
        auto result = find_last_of_ex(input, std::string_view::npos, std::forward<Keys>(keys)...);

        return std::get<0>(result);
    }

	template <typename ContainerType, typename EndType, typename Key>
    auto find_furthest_ex(const ContainerType& input, EndType&& end_value, Key&& key)
    {
        return find(input, std::forward<EndType>(end_value), std::forward<Key>(key));
    }

    template <typename ContainerType, typename EndType, typename Key, typename ...Keys>
    auto find_furthest_ex(const ContainerType& input, EndType&& end_value, Key&& key, Keys&&... keys)
    {
        auto current = find_furthest_ex(input, end_value, std::forward<Key>(key));

        if constexpr (sizeof...(Keys) == 0)
        {
            return current;
        }
        else
        {
            constexpr bool is_tuple_return_value = find_returns_tuple_v<Key>; // !std::is_same_v<std::decay_t<EndType>, std::decay_t<decltype(current)>>;

            bool current_is_valid = false;

            if constexpr (is_tuple_return_value)
            {
                current_is_valid = (std::get<0>(current) != end_value);
            }
            else
            {
                current_is_valid = (current != end_value);
            }

            auto next = find_furthest_ex(input, std::forward<EndType>(end_value), std::forward<Keys>(keys)...);

            if (current_is_valid)
            {
                if constexpr (is_tuple_return_value)
                {
					if (std::get<0>(next) != end_value)
					{
						if (std::get<0>(next) > std::get<0>(current))
						{
							return next;
						}
					}
                }
                else
                {
					if (next != end_value)
					{
						if (next > current)
						{
							return next;
						}
					}
                }

				return current;
            }

            return next;
        }
    }

    template <typename ContainerType, typename ...Keys>
    auto find_furthest(const ContainerType& input, Keys&&... keys)
    {
        auto result = find_furthest_ex(input, input.end(), std::forward<Keys>(keys)...);

        return std::get<0>(result);
    }

    template <typename ...Keys>
    std::size_t find_furthest(const std::string& input, Keys&&... keys)
    {
        auto result = find_furthest_ex(input, std::string::npos, std::forward<Keys>(keys)...);

        return std::get<0>(result);
    }

    template <typename ...Keys>
    std::size_t find_furthest(const std::string_view& input, Keys&&... keys)
    {
        auto result = find_furthest_ex(input, std::string_view::npos, std::forward<Keys>(keys)...);

        return std::get<0>(result);
    }

	// TODO: Implement generic (non-string) versions of this algorithm.
	// 
	// Executes `find_fn` repeatedly until a match is no longer found.
	// The newest match, if any, will be returned upon completion.
	// 
	// The `find_fn` callable takes in an `std::string_view` (representing a sub-string of `expr`),
	// and is expected to return a `std::tuple<std::size_t, std::string_view>`.
	template <typename FindFn>
	std::tuple<std::size_t, std::string_view> find_last_ex(std::string_view expr, FindFn&& find_fn)
	{
		std::size_t latest_result = std::string_view::npos;
		std::string_view latest_symbol = {};

		std::size_t position = 0; // offset

		while (position < expr.length())
		{
			auto result = find_fn(expr.substr(position));

			const auto& sub_expr_symbol_position = std::get<0>(result);

			if (sub_expr_symbol_position == std::string_view::npos)
			{
				break;
			}

			const auto& next_symbol = std::get<1>(result);

			if (next_symbol.empty())
			{
				break;
			}

			const auto next_symbol_position = (sub_expr_symbol_position + position);

			latest_result = next_symbol_position;
			latest_symbol = next_symbol;

			position = (next_symbol_position + next_symbol.length());
		}

		return { latest_result, latest_symbol };
	}

	// TODO: Implement generic (non-string) versions of this algorithm.
	// 
	// Executes `find_furthest_ex` on `keys` until a match can
	// no longer be found, then returns the latest match.
	template <typename ...Keys>
	std::tuple<std::size_t, std::string_view> find_last(std::string_view expr, const Keys&... keys)
	{
		return find_last_ex
		(
			expr,

			[&](std::string_view sub_expr) -> std::tuple<std::size_t, std::string_view>
			{
				//static_assert(find_returns_tuple_v<decltype(sub_expr)>);

				return find_furthest_ex
				(
					sub_expr,

					std::string_view::npos,

					// NOTE: Forward considered safe due to const-reference requirement from `find_furthest_ex`.
					keys...
				);
			}
		);
	}
}