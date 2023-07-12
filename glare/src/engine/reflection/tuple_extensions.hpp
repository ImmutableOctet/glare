#pragma once

#include <type_traits>
#include <utility>
#include <tuple>

namespace engine::impl
{
	template
	<
		std::size_t index,
		typename TupleType//,
		//typename ElementType = std::tuple_element_t<index, TupleType>,
		//typename = std::enable_if_t<std::is_copy_constructible_v<std::remove_cvref_t<ElementType>>>
	>
	auto get_tuple_element(const TupleType& instance) // -> decltype(auto) // -> ElementType
	{
		return std::get<index>(instance);
	}

	template <std::size_t index, typename TupleType>
	TupleType& set_tuple_element
	(
		TupleType& instance,
		const std::tuple_element_t<index, TupleType>& value
	) // -> decltype(auto)
	{
		static_assert(std::is_copy_assignable_v<std::remove_cvref_t<decltype(value)>>);

		auto& element = std::get<index>(instance);

		element = value;

		return instance;
	}
}