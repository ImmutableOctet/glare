#pragma once

//#include <map>
//#include <unordered_map>

namespace util
{
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
}