#pragma once

#include <algorithm>
#include <array>
#include <tuple>
#include <utility>
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

	// Returns an iterator to an entry found in `input` matching one of the keys specified.
	template <typename MapType, typename Key>
	auto find_any(const MapType& input, Key&& key)
	{
		return input.find(key);
	}

	// Returns an iterator to an entry found in `input` matching one of the keys specified.
	template <typename MapType, typename Key, typename ...Keys>
	auto find_any(const MapType& input, Key&& key, Keys&&... keys)
	{
		if (auto it = find_any(input, key); it != input.end())
		{
			return it;
		}

		return find_any(input, std::forward<Keys>(keys)...);
	}
}