#pragma once

#include <algorithm>

#include <array>
#include <utility>
#include <tuple>
#include <optional>

namespace util
{
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
}