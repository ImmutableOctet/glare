#pragma once

#include <type_traits>

#include "function_traits.hpp"
#include "member_traits.hpp"

namespace util
{
	template <typename ReturnType, typename Callable, typename ...Args>
	constexpr bool is_return_value = std::is_same_v<std::invoke_result_t<Callable, Args...>, ReturnType>;
}