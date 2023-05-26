#pragma once

namespace util
{
	// Compares `key` to each value in `compare`, returning true if a match is found.
	template <typename ValueType, typename ...ComparisonValues>
	bool compare_values(ValueType&& key, ComparisonValues&&... compare)
	{
		if (sizeof...(compare) == 0)
		{
			return false;
		}

		return ((key == compare) || ...);
	}

	template <typename TransformFn, typename ValueType, typename ...ComparisonValues>
	bool compare_values_transformed(TransformFn&& transform_fn, ValueType&& key, ComparisonValues&&... compare)
	{
		if (sizeof...(compare) == 0)
		{
			return false;
		}

		return ((transform_fn(key) == transform_fn(compare)) || ...);
	}
}