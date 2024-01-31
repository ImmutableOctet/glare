#pragma once

#include "bitflag.hpp"

#include "type_traits.hpp"

#include <cstdint>
#include <climits>
#include <type_traits>
#include <utility>

namespace util
{
	template <typename InType, typename OutType>
	inline OutType convert_flag(InType current_in, OutType current_out, InType check_in, OutType flag_out)
	{
		if ((current_in & check_in))
		{
			current_out |= flag_out;
		}

		return current_out;
	}

	template
	<
		typename T,
	
		bool reverse=false,
	
		typename BitIndexType=integral_or_size_t<T>,
		typename BitValueType=bool,

		typename Callback
	>
	constexpr void enumerate_bits(T&& value, Callback&& callback)
	{
		constexpr auto n_bits = static_cast<BitIndexType>(sizeof(value) * CHAR_BIT);

		auto on_bit = [&](BitIndexType bit_index)
		{
			using TValueType = std::remove_reference_t<T>;

			const auto mask = static_cast<TValueType>(static_cast<TValueType>(1) << static_cast<TValueType>(bit_index));
		
			const auto bit_value = static_cast<BitValueType>(value & mask);

			if constexpr (std::is_invocable_r_v<bool, Callback, decltype(mask), decltype(bit_index), decltype(bit_value)>)
			{
				if (!callback(mask, bit_index, bit_value))
				{
					return false;
				}
			}
			else if constexpr (std::is_invocable_v<Callback, decltype(mask), decltype(bit_index), decltype(bit_value)>)
			{
				callback(mask, bit_index, bit_value);
			}
			else if constexpr (std::is_invocable_r_v<bool, Callback, decltype(bit_index), decltype(bit_value)>)
			{
				if (!callback(bit_index, bit_value))
				{
					return false;
				}
			}
			else if constexpr (std::is_invocable_v<Callback, decltype(bit_index), decltype(bit_value)>)
			{
				callback(bit_index, bit_value);
			}

			return true;
		};

		if constexpr (reverse)
		{
			for (auto bit_index = BitIndexType {}; bit_index < n_bits; bit_index++)
			{
				if (!on_bit(n_bits - bit_index - static_cast<BitIndexType>(1)))
				{
					break;
				}
			}
		}
		else
		{
			for (auto bit_index = BitIndexType {}; bit_index < n_bits; bit_index++)
			{
				if (!on_bit(bit_index))
				{
					break;
				}
			}
		}
	}

	template
	<
		typename T,

		bool reverse=false,

		typename BitIndexType=T,
		typename FilterValueType=bool,

		typename Callback
	>
	constexpr void enumerate_bits_by_value(T&& value, const FilterValueType& intended_value, Callback&& callback)
	{
		enumerate_bits<T, reverse, BitIndexType, FilterValueType>
		(
			std::forward<T>(value),

			[&callback, &intended_value](BitIndexType bit_index, FilterValueType bit_value) -> bool
			{
				if (bit_value == intended_value)
				{
					if constexpr (std::is_invocable_r_v<bool, Callback, decltype(bit_index), decltype(bit_value)>)
					{
						return callback(bit_index, bit_value);
					}
					else if constexpr (std::is_invocable_v<Callback, decltype(bit_index), decltype(bit_value)>)
					{
						callback(bit_index, bit_value);
					}
					else if constexpr (std::is_invocable_r_v<bool, Callback, decltype(bit_index)>)
					{
						return callback(bit_index);
					}
					else if constexpr (std::is_invocable_v<Callback, decltype(bit_index)>)
					{
						callback(bit_index);
					}
				}

				return true;
			}
		);
	}

	template <typename T, typename BitIndexType=std::remove_reference_t<T>, typename FilterValueType=bool>
	constexpr std::size_t count_bits_by_value(T&& value, const FilterValueType& intended_value)
	{
		auto bits_found = std::size_t {};

		enumerate_bits_by_value<T, false, BitIndexType, FilterValueType>
		(
			std::forward<T>(value),
			intended_value,

			[&bits_found](auto&&...)
			{
				bits_found++;
			}
		);

		return bits_found;
	}

	template <typename T>
	constexpr std::size_t count_enabled_bits(T&& value)
	{
		return count_bits_by_value(std::forward<T>(value), true);
	}

	template <typename T>
	constexpr std::size_t count_disabled_bits(T&& value)
	{
		return count_bits_by_value(std::forward<T>(value), false);
	}

	template
	<
		typename T,
		typename BitIndexType,

		bool intended_value,
		bool reverse,
	
		typename Callback
	>
	constexpr std::size_t enumerate_boolean_bits_impl(T&& value, Callback&& callback)
	{
		auto bits_found = std::size_t {};

		enumerate_bits_by_value<T, reverse, BitIndexType, bool>
		(
			std::forward<T>(value),

			intended_value,

			[&callback, &bits_found](BitIndexType index, bool value) -> bool
			{
				if constexpr (std::is_invocable_r_v<bool, Callback, BitIndexType, decltype(intended_value)>)
				{
					if (!callback(index, value))
					{
						return false;
					}
				}
				else if constexpr (std::is_invocable_v<Callback, BitIndexType, decltype(intended_value)>)
				{
					callback(index, value);
				}
				else if constexpr (std::is_invocable_r_v<bool, Callback, BitIndexType>)
				{
					if (!callback(index))
					{				
						return false;
					}
				}
				else if constexpr (std::is_invocable_v<Callback, BitIndexType>)
				{
					callback(index);
				}

				bits_found++;

				return true;
			}
		);

		return bits_found;
	}

	template
	<
		bool reverse=false,

		typename T,
		typename BitIndexType=integral_or_size_t<T>,
	
		typename Callback
	>
	constexpr std::size_t enumerate_enabled_bits(T&& value, Callback&& callback)
	{
		return enumerate_boolean_bits_impl<T, BitIndexType, true, reverse, Callback>(std::forward<T>(value), std::forward<Callback>(callback));
	}

	template
	<
		bool reverse=false,

		typename T,
		typename BitIndexType=integral_or_size_t<T>,

		typename Callback
	>
	constexpr std::size_t enumerate_disabled_bits(T&& value, Callback&& callback)
	{
		return enumerate_boolean_bits_impl<T, BitIndexType, false, reverse, Callback>(std::forward<T>(value), std::forward<Callback>(callback));
	}

	static_assert(count_disabled_bits(0b1110) == ((sizeof(0b1110) * CHAR_BIT) - 3));
	static_assert(count_enabled_bits(0b111110) == 5);
	static_assert(enumerate_enabled_bits(0b1010101111111, [](auto&&...) {}) == 10);
	static_assert(enumerate_disabled_bits(0b111010, [](auto&&...) {}) == ((sizeof(0b111010) * CHAR_BIT) - 4));
}