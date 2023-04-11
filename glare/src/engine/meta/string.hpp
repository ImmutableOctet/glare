#pragma once

#include "types.hpp"

#include <string>
#include <string_view>

namespace engine
{
	bool meta_any_is_string(const MetaAny& value);
    bool meta_type_is_string(const MetaType& type);

	// Attempts to compute a string hash for `value`.
	std::optional<StringHash> meta_any_to_string_hash
    (
        const MetaAny& value,
        
        bool try_raw_hash_type=true,
        bool try_conversion_to_raw_hash_type=false,
        bool try_conversion_to_string=false
    );

	// Compares `left` and `right` to see if they both have the same string hash.
	bool meta_any_string_compare(const MetaAny& left, const MetaAny& right);

    template <typename Callback>
	bool try_string_value(const MetaAny& value, Callback&& callback)
	{
		if (try_value<std::string>(value, callback))
		{
			return true;
		}

		if (try_value<std::string_view>(value, callback))
		{
			return true;
		}

		return false;
	}
}