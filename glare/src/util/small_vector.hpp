#pragma once

// Needed on latest MSVC versions when including Folly's `small_vector` type,
// since it uses `std::aligned_storage` and related deprecated functionality.
//#define _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING 1

#include <folly/small_vector.h>

//#undef _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING

namespace util
{
	using folly::small_vector;
}