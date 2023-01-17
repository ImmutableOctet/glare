#pragma once

#include <type_traits>

#include "function_traits.hpp"
#include "member_traits.hpp"

namespace util
{
	template <typename ReturnType, typename Callable, typename ...Args>
	constexpr bool is_return_value = std::is_same_v<std::invoke_result_t<Callable, Args...>, ReturnType>;

	template<typename T>
	struct parent_type_from_member_pointer;

	template<typename Type, typename MemberType>
	struct parent_type_from_member_pointer<MemberType Type::*>
	{
		using type = Type;
		
		using parent_type = Type;
		using member_type = MemberType;
	};

	template<typename T>
	struct member_pointer_type;

	template<typename Type, typename MemberType>
	struct member_pointer_type<MemberType Type::*>
	{
		using type = MemberType;

		using parent_type = Type;
		using member_type = MemberType;
	};
}