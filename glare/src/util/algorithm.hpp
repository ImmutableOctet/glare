#pragma once

#include "type_traits.hpp"
#include "min_max.hpp"

#include <type_traits>
#include <algorithm>
#include <string_view>
#include <string>
#include <array>
#include <tuple>
#include <utility>
#include <optional>

namespace util
{
	namespace impl
    {
        template <typename Key>
        inline constexpr bool key_has_value_semantics_v = (std::is_copy_constructible_v<Key> || std::is_move_constructible_v<Key>);

        template <typename Key>
        inline constexpr bool is_regular_key_v = (std::is_default_constructible_v<Key> && key_has_value_semantics_v<Key>);

        template <typename Key>
        inline constexpr bool key_is_optional_compatible_v = ((!is_regular_key_v<Key>) && (key_has_value_semantics_v<Key>) && is_optional_compatible_v<Key>);

        template <typename Key>
        inline constexpr bool find_returns_tuple_v = (is_regular_key_v<Key> || is_c_str_v<Key> || (key_has_value_semantics_v<Key> && key_is_optional_compatible_v<Key>));

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

	template<std::size_t index, typename... Types>
	using get_type_by_index = typename std::tuple_element<index, std::tuple<Types...>>::type;

	static_assert(std::is_same_v<get_type_by_index<2, int, float, double>, double>);
	static_assert(std::is_same_v<std::decay_t<get_type_by_index<0, int&, float>>, int>);

	template<typename... Types>
	using get_first_type = get_type_by_index<0, Types...>;

	static_assert(std::is_same_v<get_first_type<short, double, int>, short>);

	template <typename T>
	inline bool equal_or_nullopt(const std::optional<T>& value, T&& comparison)
	{
		//return ((!value.has_value()) || (*value == comparison));


		if (!value.has_value())
		{
			return true;
		}

		return (*value == comparison);
	}

	template <typename ValueType, typename MapType, typename KeyType>
	inline std::optional<ValueType> get_if_exists(const MapType& container, KeyType&& key)
	{
		auto it = container.find(key);

		if (it != container.end())
		{
			return it->second;
		}

		return std::nullopt;
	}

	// Returns a T& to the value held in 'opt', if one exists.
	// If no value is present, then 'fallback' is returned instead.
	template <typename T>
	inline T& get_mutable(std::optional<T>& opt, T& fallback)
	{
		if (opt.has_value())
		{
			return *opt;
		}

		return fallback;
	}

	// Same as 'get_mutable', but allows for 'nullptr' on 'fallback'.
	template <typename T>
	inline T* get_mutable(std::optional<T>& opt, T* fallback=nullptr)
	{
		if (opt.has_value())
		{
			return &(*opt); // Pointer to 'T'.
		}

		return fallback;
	}

	/*
	template <class Target = void, class... TupleLike>
	auto concatenate(TupleLike&&... tuples)
	{
		return std::apply
		(
			[](auto&& first, auto&&... rest)
			{
				using T = std::conditional_t<!std::is_void<Target>::value, Target, std::decay_t<decltype(first)>>;

				return std::array<T, sizeof...(rest) + 1> { { decltype(first)(first), decltype(rest)(rest)... } };
			},

			std::tuple_cat(std::forward<TupleLike>(tuples)...)
		);
	}
	*/

	template <typename lhs_t, typename rhs_t>
	constexpr auto concatenate(lhs_t lhs, rhs_t rhs)
	{
		std::array<typename lhs_t::value_type, (lhs.size() + rhs.size())> result {};

		// TODO: Review use of unsigned integer for indexing.
		std::size_t index = 0;

		for (auto& element : lhs)
		{
			result[index] = std::move(element);

			index++;
		}

		for (auto& element : rhs)
		{
			result[index] = std::move(element);

			index++;
		}

		return result;
	}

	template <typename ResultType, typename ContainerType, typename GetFn, typename DivisionFn>
	ResultType get_avg_value
	(
		const ContainerType& container,
		std::size_t n_samples,

		GetFn&& get_fn,
		DivisionFn&& div_fn,

		std::size_t offset=0
	)
	{
		//auto origin = (previous.begin() + offset);
		//return (std::reduce(origin, (origin + n_samples)) / n_samples);

		auto it = (container.begin() + offset);

		// NOTE: Assert disabled since MSVC's STL has a better diagnostic.
		//assert((it + n_samples) <= container.end());

		auto dest = (it + n_samples);

		ResultType sum = {};

		//std::size_t samples_collected = 0;

		for (; it < dest; it++)
		{
			sum += get_fn(*it);

			//samples_collected++;
		}

		//return (sum / n_samples);

		return div_fn(sum, n_samples); // samples_collected
	}

	template <std::size_t tuple_index, typename ContainerType, typename DivisionFn>
	auto get_avg_value_from_tuple_samples
	(
		const ContainerType& container,
		std::size_t n_samples,
		DivisionFn&& div_fn,

		std::size_t offset=0
	)
	{
		using TupleType  = typename ContainerType::value_type;
		using ResultType = std::tuple_element<tuple_index, TupleType>::type;

		return get_avg_value<ResultType, ContainerType>
		(
			container, n_samples,
			[](const auto& sample) -> ResultType
			{
				return std::get<tuple_index>(sample);
			},
			div_fn,
			offset
		);
	}

	template <std::size_t tuple_index, typename ContainerType>
	auto get_avg_value_from_tuple_samples
	(
		const ContainerType& container,
		std::size_t n_samples,
		std::size_t offset=0
	)
	{
		return get_avg_value_from_tuple_samples<tuple_index>
		(
			container, n_samples,
			[](auto&& value, auto&& divisor)
			{
				return (value / divisor);
			},
			offset
		);
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
            else if constexpr (is_c_str_v<Key>)
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
        else if constexpr (is_c_str_v<Key>)
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

	// Compares `key` to each value in `compare`, returning true if a match is found.
	template <typename ValueType, typename ...ComparisonValues>
	bool compare_values(ValueType&& key, ComparisonValues&&... compare)
	{
		if (sizeof...(compare) == 0)
		{
			return false;
		}

		return ((key == compare) || ...);
	}

	template <typename TransformFn, typename ValueType, typename ...ComparisonValues>
	bool compare_values_transformed(TransformFn&& transform_fn, ValueType&& key, ComparisonValues&&... compare)
	{
		if (sizeof...(compare) == 0)
		{
			return false;
		}

		return ((transform_fn(key) == transform_fn(compare)) || ...);
	}

	template <typename MapType, typename KeyTransformFn, typename Callback, typename ...Ignore>
	std::size_t enumerate_map_filtered_ex(MapType&& container, KeyTransformFn&& key_transform_fn, Callback&& callback, Ignore&&... ignore)
	{
		std::size_t count = 0;

		for (auto&& [first, second] : container)
		{
			if (compare_values_transformed(key_transform_fn, first, std::forward<Ignore>(ignore)...))
			{
				continue;
			}

			count++;

			using first_t  = decltype(first);
			using second_t = decltype(second);

			if constexpr (is_return_value<bool, Callback, first_t, second_t>)
			{
				if (!callback(std::forward<first_t>(first), std::forward<second_t>(second)))
				{
					break;
				}
			}
			else
			{
				callback(std::forward<first_t>(first), std::forward<second_t>(second));
			}
		}

		return count;
	}

	// Enumerates `container`, executing `callback` for each pair where the key does not match any of the items in `ignore`.
	template <typename MapType, typename Callback, typename ...Ignore>
	std::size_t enumerate_map_filtered(MapType&& container, Callback&& callback, Ignore&&... ignore)
	{
		return enumerate_map_filtered_ex(container, [](auto&& value) { return value; }, callback, std::forward<Ignore>(ignore)...);
	}

	template <typename Callback, std::size_t... Indices>
	auto generate_indices(Callback&& callback, std::index_sequence<Indices...>)
	{
		return callback(std::integral_constant<std::size_t, Indices>()...);
	}

	template <std::size_t n_elements, typename Callback>
	auto generate_indices(Callback&& callback)
	{
		return generate_indices(callback, std::make_index_sequence<n_elements>());
	}

	template <std::size_t n_from_begin, std::size_t n_from_end, typename Callback, typename ...Args>
	auto call_truncated(Callback&& callback, Args&&... args)
	{
		return generate_indices<(sizeof...(Args) - (n_from_begin + n_from_end))>
		(
			[&](auto... Indices)
			{
				//auto args_truncated = std::make_tuple(std::forward<Args>(args)...);
				auto args_truncated = std::forward_as_tuple<Args...>(std::forward<Args>(args)...);

				return callback(std::get<Indices + n_from_begin>(args_truncated)...);
			}
		);
	}

	template <std::size_t n_from_begin, typename Callback, typename ...Args>
	auto drop_first_n(Callback&& callback, Args&&... args)
	{
		return call_truncated<n_from_begin, 0>(std::forward<Callback>(callback), std::forward<Args>(args)...);
	}

	template <typename Callback, typename ...Args>
	auto drop_first(Callback&& callback, Args&&... args)
	{
		return drop_first_n<1>(std::forward<Callback>(callback), std::forward<Args>(args)...);
	}

	template <std::size_t n_from_end, typename Callback, typename ...Args>
	auto drop_last_n(Callback&& callback, Args&&... args)
	{
		return call_truncated<0, n_from_end>(std::forward<Callback>(callback), std::forward<Args>(args)...);
	}

	template <typename Callback, typename ...Args>
	auto drop_last(Callback&& callback, Args&&... args)
	{
		return drop_last_n<1>(std::forward<Callback>(callback), std::forward<Args>(args)...);
	}

	template <std::size_t n_from_right, typename Callback, typename ...Args>
	auto drop_all_but_n_from_right(Callback&& callback, Args&&... args)
	{
		return call_truncated<(sizeof...(Args) - n_from_right), 0>(std::forward<Callback>(callback), std::forward<Args>(args)...);
	}

	template <std::size_t n_from_left, typename Callback, typename ...Args>
	auto drop_all_but_n_from_left(Callback&& callback, Args&&... args)
	{
		return call_truncated<0, (sizeof...(Args) - n_from_left)>(std::forward<Callback>(callback), std::forward<Args>(args)...);
	}

	template <typename Callback, typename ...Args>
	auto drop_all_but_first(Callback&& callback, Args&&... args)
	{
		return drop_all_but_n_from_left<1>(std::forward<Callback>(callback), std::forward<Args>(args)...);
	}

	template <typename Callback, typename ...Args>
	auto drop_all_but_last(Callback&& callback, Args&&... args)
	{
		return drop_all_but_n_from_right<1>(std::forward<Callback>(callback), std::forward<Args>(args)...);
	}

	/*
		Executes `callback` using `args`, dropping the last element of `args` on each failed execution.
		The return-value of `callback` must be convertible to boolean, but can be any type otherwise.

		The `min_arg_count` template parameter controls the minimum
		number of elements that can be safely passed to `callback`.
		
		If a successful result is not found by the end of the final (`min_arg_count`) execution of `callback`,
		the returned value from the final execution will be forwarded to the caller.

		If a successful result is found at any point, this function will immediately return.
		
		Example:
			* callback(args[0], args[1], args[2], args[3]) // Failure
			  -> callback(args[0], args[1], args[2])       // Failure
			    -> callback(args[0], args[1])              // Success
			      -> Return value
	*/
	template <std::size_t min_arg_count, typename ...Args>
	auto drop_last_until_success(auto&& callback, Args&&... args)
	{
		auto result = callback(args...); // std::forward<Args>(args)...

		if (result)
		{
			return result;
		}
		
		if constexpr (sizeof...(args) > (min_arg_count))
		{
			return drop_last
			(
				[&callback](auto&&... truncated_args)
				{
					return drop_last_until_success<min_arg_count>
					(
						callback,

						std::forward<decltype(truncated_args)>(truncated_args)...
					);
				},

				std::forward<Args>(args)...
			);
		}
		else
		{
			return result;
		}
	}

	template <std::size_t index, typename ...Args>
	auto&& get(Args&&... args)
	{
		return std::get<index>(std::forward_as_tuple<Args...>(std::forward<Args>(args)...));
	}

	template <typename ...Args>
	auto&& get_first(Args&&... args)
	{
		//if constexpr (sizeof...(args) > 0)
		{
			return get<0>(std::forward<Args>(args)...);
		}
	}

	template <typename ...Args>
	auto&& get_last(Args&&... args)
	{
		//if constexpr (sizeof...(args) > 0)
		{
			return get<(sizeof...(args) - 1)>(std::forward<Args>(args)...);
		}
	}
}