#pragma once

#include "basic_vector_queue.hpp"

#include "small_vector.hpp"

#include <cstddef>

namespace util
{
	template <typename T, std::size_t RequestedMaxInline=1, typename ...Policies>
	using small_vector_queue = basic_vector_queue<small_vector<T, RequestedMaxInline, Policies...>>;
}