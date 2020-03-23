#pragma once

#include <variant>
#include <optional>

namespace util
{
	// Calls 'fn' with a value of type 'T' if 'v' contains said value.
	template <typename T, typename VariantType, typename FunctionType>
	bool peek_value(const VariantType& v, FunctionType fn)
	{
		if (const auto value = std::get_if<T>(&v))
		{
			fn(*value);

			return true;
		}

		return false;
	}

	template <typename T, typename VariantType>
	T get_value(const VariantType& v, const T& default_value)
	{
		T value_out = default_value;

		peek_value<T>(v, [value_out](const auto& value) mutable
		{
			value_out = value;
		});

		return value_out;
	}

	template <typename T, typename VariantType>
	T get_value_safe(const VariantType& v, const T& default_value={})
	{
		return get_value(v, default_value);
	}

	template <typename T, typename VariantType>
	std::optional<T> get_optional(const VariantType& v)
	{
		T value_out;

		if (peek_value(v, [value_out](const auto& value) mutable { value_out = value; }))
		{
			return value_out;
		}
		else
		{
			return std::nullopt;
		}
	}
}