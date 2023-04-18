#pragma once

#include "tuple.hpp"
#include "type_traits.hpp"

#include <type_traits>
#include <utility>
#include <tuple>

namespace util
{
	template<std::size_t index, typename... Types>
	using get_type_by_index = typename std::tuple_element<index, std::tuple<Types...>>::type;

	static_assert(std::is_same_v<get_type_by_index<2, int, float, double>, double>);
	static_assert(std::is_same_v<std::decay_t<get_type_by_index<0, int&, float>>, int>);

	template<typename... Types>
	using get_first_type = get_type_by_index<0, Types...>;

	static_assert(std::is_same_v<get_first_type<short, double, int>, short>);

	// Calls `stateless_generic_lambda` with the types in `Ts`,
	// removing types from the end of the sequence until none are left.
	template <auto stateless_generic_lambda, typename... Ts>
	constexpr void deplete_types()
	{
		deplete_tuple_types<stateless_generic_lambda, std::tuple<Ts...>>();
	}

	// Calls `lambda_instance` with the types in `Ts`,
	// removing types from the end of the sequence until none are left.
	template <typename GenericLambda, typename... Ts>
	constexpr void deplete_types(GenericLambda&& lambda_instance)
	{
		deplete_tuple_types<std::tuple<Ts...>, GenericLambda, 0>(std::forward<GenericLambda>(lambda_instance));
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
	constexpr auto&& get(Args&&... args)
	{
		return std::get<index>(std::forward_as_tuple<Args...>(std::forward<Args>(args)...));
	}

	template <typename ...Args>
	constexpr auto&& get_first(Args&&... args)
	{
		//if constexpr (sizeof...(args) > 0)
		{
			return get<0>(std::forward<Args>(args)...);
		}
	}

	template <typename ...Args>
	constexpr auto&& get_last(Args&&... args)
	{
		//if constexpr (sizeof...(args) > 0)
		{
			return get<(sizeof...(args) - 1)>(std::forward<Args>(args)...);
		}
	}

	template <typename T, typename Fn, typename... Args>
	constexpr decltype(auto) execute_as(Fn&& fn, Args&&... args)
	{
		if constexpr (std::is_same_v<void, decltype(fn(T{std::forward<Args>(args)}...))>)
		{
			fn(T { std::forward<Args>(args) }...);
		}
		else
		{
			return fn(T { std::forward<Args>(args) }...);
		}
	}

	template <typename T, typename Fn, typename... TupleArgs>
	constexpr decltype(auto) execute_tuple_as(Fn&& fn, TupleArgs&&... args)
	{
		return std::apply
		(
			[&fn](auto&&... values)
			{
				if constexpr (std::is_same_v<void, decltype(execute_as<T>(fn, std::forward<decltype(values)>(values)...))>)
				{
					execute_as<T>(fn, std::forward<decltype(values)>(values)...);
				}
				else
				{
					return execute_as<T>(fn, std::forward<decltype(values)>(values)...);
				}
			},

			std::forward<TupleArgs>(args)...
		);
	}

	template <typename T, typename Fn, typename... Values>
	constexpr bool inspect_as(Fn&& fn, Values&&... values)
	{
		bool result = true;

		std::apply
		(
			[&fn, &result](auto&&... inputs)
			{
				if constexpr ((is_convertible_to_bool<decltype(fn(T { std::forward<decltype(inputs)>(inputs) }))> && ...))
				{
					result = static_cast<bool>((fn(T { std::forward<decltype(inputs)>(inputs) }) && ...));
				}
				else
				{
					(
						fn(T { std::forward<decltype(inputs)>(inputs) }),
						...
					);
				}
			},

			std::forward_as_tuple<Values...>(std::forward<Values>(values)...)
		);

		return result;
	}

	template <typename T, typename Fn, typename... TupleArgs>
	constexpr decltype(auto) inspect_tuple_as(Fn&& fn, TupleArgs&&... args)
	{
		return execute_tuple_as<T>
		(
			[&fn](auto&&... tuple_entries)
			{
				return (inspect_as<T>(fn, std::forward<decltype(tuple_entries)>(tuple_entries)) && ...);
			},

			std::forward<TupleArgs>(args)...
		);
	}

	static_assert
	(
		inspect_as<int>
		(
			[](auto value) { return std::is_same_v<decltype(value), int>; },
			static_cast<short>(0)
		)
	);

	static_assert
	(
		inspect_tuple_as<int>
		(
			[](auto value) { return std::is_same_v<decltype(value), int>; },
			std::make_tuple<short>(static_cast<short>(0))
		)
	);

	static_assert
	(
		execute_as<int>
		(
			[](auto value) { return std::is_same_v<decltype(value), int>; },
			static_cast<short>(0)
		)
	);

	static_assert
	(
		execute_tuple_as<int>
		(
			[](auto value) { return (std::is_same_v<decltype(value), int>); },
			std::make_tuple<short>(static_cast<short>(0))
		)
	);
}