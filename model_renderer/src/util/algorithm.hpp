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