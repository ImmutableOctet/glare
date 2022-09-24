#pragma once

#include <optional>

namespace util
{
	template <typename T, typename FallbackFn>
	T value_or(const std::optional<T>& opt, FallbackFn&& fallback)
	{
		if (opt.has_value())
		{
			return *opt;
		}

		return fallback();
	}

	template <typename T, typename TrueFn>
	constexpr std::optional<T> optional_get(bool condition, TrueFn&& fn_if_true)
	{
		if (condition)
		{
			return fn_if_true();
		}

		return std::nullopt;
	}

	template <typename T, typename ConditionType, typename TrueFn>
	constexpr std::optional<T> optional_inspect(ConditionType condition, TrueFn&& fn_if_true)
	{
		if (condition)
		{
			return fn_if_true(condition);
		}

		return std::nullopt;
	}

	// Workaround for `std::nullopt` not working in unary-if statements.
	template <typename T>
	constexpr std::optional<T> optional_if(bool condition, T&& true_value)
	{
		if (condition)
		{
			return true_value;
		}

		return std::nullopt;
	}

	// Similar to other overload of `optional_if`, but allows the user to specify the `false_value`.
	template <typename T> // template <typename true_t, typename false_t>
	constexpr std::optional<T> optional_if(bool condition, T&& true_value, T&& false_value)
	{
		//return ((condition) ? true_value : false_value);

		if (condition)
		{
			return true_value;
		}

		return false_value;
	}

	// Similar to other overload of `optional_if`, but allows the user to specify the `false_value` as another optional.
	template <typename T> // template <typename true_t, typename false_t>
	constexpr std::optional<T> optional_if(bool condition, T&& true_value, std::optional<T>&& false_value)
	{
		if (condition)
		{
			return true_value;
		}

		return false_value;
	}
}