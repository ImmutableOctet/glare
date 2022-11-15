#pragma once

#include <util/member_function_traits.hpp>

#include <type_traits>

namespace engine
{
	// Useful for integrating systems:
	GENERATE_HAS_METHOD_TRAIT(subscribe);   // `has_method_subscribe`
	GENERATE_HAS_METHOD_TRAIT(unsubscribe); // `has_method_unsubscribe`

	// Allows for types to (optionally) define their own `reflect` static member-functions.
	// See also: `engine::reflect` free-function. (found in `reflection`)
	GENERATE_HAS_FUNCTION_TRAIT(reflect); // `has_function_reflect`
}