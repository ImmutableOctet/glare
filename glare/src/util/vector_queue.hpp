#pragma once

#include "basic_vector_queue.hpp"

#include <vector>

namespace util
{
	template <typename T>
	using vector_queue = basic_vector_queue<std::vector<T>>;
}