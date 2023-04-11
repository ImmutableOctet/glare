#pragma once

#include "types.hpp"

namespace engine
{
	// Attempts to convert `current_value` to `cast_type` in-place.
    // If the type-cast was successful, this will return true.
    // If `current_value` is an indirect value, this will fail (return false).
    bool try_direct_cast(MetaAny& current_value, const MetaType& cast_type, bool ignore_same_type=true);
}