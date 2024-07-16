#pragma once

#include <util/type_traits.hpp>

#include <type_traits>

#include <string>
#include <string_view>

namespace engine
{
	template <typename T>
	inline static constexpr bool is_string_type_v =
	(
		(std::is_same_v<std::remove_cvref_t<T>, std::string>)
		||
		(std::is_same_v<std::remove_cvref_t<T>, std::string_view>)
		||
		(util::is_c_str_v<T>)
	);
}