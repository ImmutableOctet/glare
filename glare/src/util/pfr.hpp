#pragma once

#include <boost/pfr.hpp>

#include <type_traits>

namespace util
{
	namespace pfr = boost::pfr;

	template <typename T>
	inline constexpr bool is_pfr_reflective_v = static_cast<bool>(pfr::tuple_size_v<std::remove_reference_t<T>>);
}