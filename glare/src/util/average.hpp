#pragma once

#include <tuple>
//#include <algorithm>

namespace util
{
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
		using ResultType = typename std::tuple_element<tuple_index, TupleType>::type;

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
}