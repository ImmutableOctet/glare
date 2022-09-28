#pragma once

#include <util/member_function_traits.hpp>

namespace engine
{
	GENERATE_HAS_METHOD_TRAIT(subscribe);   // `has_method_subscribe`
	GENERATE_HAS_METHOD_TRAIT(unsubscribe); // `has_method_unsubscribe`
}