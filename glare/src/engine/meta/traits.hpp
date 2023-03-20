#pragma once

#include <util/type_traits.hpp>
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

	GENERATE_HAS_FIELD_TRAIT(check_linked); // `has_field_check_linked`

	GENERATE_HAS_METHOD_TRAIT(get);
	GENERATE_HAS_METHOD_TRAIT(get_indirect_value);

	GENERATE_HAS_METHOD_TRAIT(set);
	GENERATE_HAS_METHOD_TRAIT(set_indirect_value);

	GENERATE_HAS_METHOD_TRAIT(has_type);
	GENERATE_HAS_METHOD_TRAIT(get_type);

	GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_plus, operator+);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_plus, operator+);
	GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_minus, operator-);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_minus, operator-);
	GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_bitwise_not, operator~);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_bitwise_not, operator~);

	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_logical_not, operator!);
	//GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_logical_not, operator!);

	GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_multiply, operator*);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_multiply, operator*);
	GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_divide, operator/);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_divide, operator/);
	GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_modulus, operator%);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_modulus, operator%);

	GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_bitwise_shift_left, operator<<);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_bitwise_shift_left, operator<<);
	GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_bitwise_shift_right, operator>>);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_bitwise_shift_right, operator>>);
	GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_bitwise_and, operator&);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_bitwise_and, operator&);
	GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_bitwise_xor, operator^);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_bitwise_xor, operator^);
	GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_bitwise_or, operator|);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_bitwise_or, operator|);

	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_less_than, operator<);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_less_than, operator<);
	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_less_than_or_equal, operator<=);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_less_than_or_equal, operator<=);
	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_greater_than, operator>);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_greater_than, operator>);
	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_greater_than_or_equal, operator>=);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_greater_than_or_equal, operator>=);

	//GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_bool, operator bool);

	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_equal, operator==);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_equal, operator==);
	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_not_equal, operator!=);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_not_equal, operator!=);

	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_assign, operator=);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_assign, operator=);

	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_add_assign, operator+=);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_add_assign, operator+=);
	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_subtract_assign, operator-=);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_subtract_assign, operator-=);
	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_multiply_assign, operator*=);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_multiply_assign, operator*=);
	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_divide_assign, operator/=);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_divide_assign, operator/=);
	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_modulus_assign, operator%=);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_modulus_assign, operator%=);

	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_bitwise_shift_left_assign, operator<<=);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_bitwise_shift_left_assign, operator<<=);
	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_bitwise_shift_right_assign, operator>>=);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_bitwise_shift_right_assign, operator>>=);
	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_bitwise_and_assign, operator&=);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_bitwise_and_assign, operator&=);
	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_bitwise_xor_assign, operator^=);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_bitwise_xor_assign, operator^=);
	//GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_operator_bitwise_or_assign, operator|=);
	GENERATE_HAS_METHOD_TRAIT_EX(has_method_operator_bitwise_or_assign, operator|=);
}