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

	template <typename T>
	constexpr std::optional<T> optional_or(const std::optional<T>& a, const std::optional<T>& b)
	{
		if (a)
		{
			return a;
		}

		return b;
	}

	template <typename T>
	bool equal_or_nullopt(const std::optional<T>& value, T&& comparison)
	{
		//return ((!value.has_value()) || (*value == comparison));

		if (!value.has_value())
		{
			return true;
		}

		return (*value == comparison);
	}

	// Returns a T& to the value held in 'opt', if one exists.
	// If no value is present, then 'fallback' is returned instead.
	template <typename T>
	T& get_mutable(std::optional<T>& opt, T& fallback)
	{
		if (opt.has_value())
		{
			return *opt;
		}

		return fallback;
	}

	// Same as 'get_mutable', but allows for 'nullptr' on 'fallback'.
	template <typename T>
	T* get_mutable(std::optional<T>& opt, T* fallback=nullptr)
	{
		if (opt.has_value())
		{
			return &(*opt); // Pointer to 'T'.
		}

		return fallback;
	}
}