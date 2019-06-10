#pragma once

#include <algorithm>

#include <array>
#include <utility>
#include <tuple>

namespace util
{
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

	template <typename T, std::size_t N1, std::size_t N2>
	constexpr std::array<T, (N1 + N2)> concatenate(std::array<T, N1> lhs, std::array<T, N2> rhs)
	{
		std::array<T, N1 + N2> result {};

		std::size_t index = 0;

		for (auto& el : lhs)
		{
			result[index] = std::move(el);

			index++;
		}
		
		for (auto& el : rhs)
		{
			result[index] = std::move(el);

			index++;
		}

		return result;
	}
}