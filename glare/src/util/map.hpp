#pragma once

#include "type_traits.hpp"
#include "compare.hpp"

//#include <map>
//#include <unordered_map>

#include <optional>

namespace util
{
	template <typename ValueType, typename MapType, typename KeyType>
	std::optional<ValueType> get_if_exists(const MapType& container, KeyType&& key)
	{
		auto it = container.find(key);

		if (it != container.end())
		{
			return it->second;
		}

		return std::nullopt;
	}

	// Searches through `container` removing pairs with the `value` specified.
	template <typename MapType, typename ValueType>
	constexpr void erase_by_value(MapType& container, ValueType&& value, bool exit_immediately=false)
	{
		for (auto pair_it = container.begin(); pair_it != container.end(); )
		{
			if (pair_it->second == value)
			{
				if (exit_immediately)
				{
					container.erase(pair_it);

					break;
				}
				else
				{
					container.erase(pair_it++);
				}
			}
			else
			{
				++pair_it;
			}
		}
	}

	template <typename MapType, typename KeyTransformFn, typename Callback, typename ...Ignore>
	std::size_t enumerate_map_filtered_ex(MapType&& container, KeyTransformFn&& key_transform_fn, Callback&& callback, Ignore&&... ignore)
	{
		std::size_t count = 0;

		for (auto&& [first, second] : container)
		{
			if (compare_values_transformed(key_transform_fn, first, std::forward<Ignore>(ignore)...))
			{
				continue;
			}

			count++;

			using first_t  = decltype(first);
			using second_t = decltype(second);

			if constexpr (is_return_value<bool, Callback, first_t, second_t>)
			{
				if (!callback(std::forward<first_t>(first), std::forward<second_t>(second)))
				{
					break;
				}
			}
			else
			{
				callback(std::forward<first_t>(first), std::forward<second_t>(second));
			}
		}

		return count;
	}

	// Enumerates `container`, executing `callback` for each pair where the key does not match any of the items in `ignore`.
	template <typename MapType, typename Callback, typename ...Ignore>
	std::size_t enumerate_map_filtered(MapType&& container, Callback&& callback, Ignore&&... ignore)
	{
		return enumerate_map_filtered_ex
		(
			container,
			[](auto&& value) { return value; },
			callback,
			std::forward<Ignore>(ignore)...
		);
	}
}