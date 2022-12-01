#pragma once

#include <util/member_traits.hpp>

#include <type_traits>

namespace engine
{
	// Useful for integrating systems:
	GENERATE_HAS_METHOD_TRAIT(subscribe);   // `has_method_subscribe`
	GENERATE_HAS_METHOD_TRAIT(unsubscribe); // `has_method_unsubscribe`

	GENERATE_HAS_METHOD_TRAIT(entity); // `has_method_entity`
	GENERATE_HAS_METHOD_TRAIT(get_entity); // `has_method_get_entity`
	GENERATE_HAS_FIELD_TRAIT(entity); // `has_field_entity`

	GENERATE_HAS_METHOD_TRAIT(service); // `has_method_service`
	GENERATE_HAS_METHOD_TRAIT(get_service); // `has_method_get_service`
	GENERATE_HAS_FIELD_TRAIT(service); // `has_field_service`

	GENERATE_HAS_METHOD_TRAIT(player); // `has_method_player`
	GENERATE_HAS_METHOD_TRAIT(get_player); // `has_method_get_player`
	GENERATE_HAS_FIELD_TRAIT(player); // `has_field_player`

	GENERATE_HAS_METHOD_TRAIT(player_index); // `has_method_player_index`
	GENERATE_HAS_METHOD_TRAIT(get_player_index); // `has_method_get_player_index`
	GENERATE_HAS_FIELD_TRAIT(player_index); // `has_field_player_index`

	// Allows for types to (optionally) define their own `reflect` static member-functions.
	// See also: `engine::reflect` free-function. (found in `reflection`)
	GENERATE_HAS_FUNCTION_TRAIT(reflect); // `has_function_reflect`
}