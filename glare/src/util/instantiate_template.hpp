#pragma once

#include "tuple.hpp"

namespace util
{
	namespace impl
	{
		template
		<
			template <typename...> typename TemplateOut,
			std::size_t offset,
			typename Sequence,
			typename... Ts
		>
		struct instantiate_template_impl;

		template
		<
			template <typename...> typename TemplateOut,
			std::size_t offset,
			std::size_t... I,
			typename... Ts
		>
		struct instantiate_template_impl<TemplateOut, offset, std::index_sequence<I...>, Ts...>
		{
			using tuple_type = std::tuple<Ts...>;
			using type = TemplateOut<typename std::tuple_element<(I + offset), tuple_type>::type...>;
		};
	}

	// Attempts to instantiate `TemplateOut` using a subset of `Ts`, specified using `offset` and `n_truncated`.
	// The resulting type may be accessed via the `type` member.
	template
	<
		template <typename...> typename TemplateOut,
		std::size_t offset,
		std::size_t n_truncated,
		typename... Ts
	>
	struct instantiate_template_ex : impl::instantiate_template_impl
	<
		TemplateOut,
		offset,
		std::make_index_sequence<((sizeof...(Ts)) - offset - n_truncated)>,
		Ts...
	> {};

	// Attempts to instantiate `TemplateOut` using `Ts`.
	// The resulting type may be accessed via the `type` member.
	template
	<
		template <typename...> typename TemplateOut,
		typename... Ts
	>
	struct instantiate_template : impl::instantiate_template_impl
	<
		TemplateOut,
		0,
		std::make_index_sequence<(sizeof...(Ts))>,
		Ts...
	> {};

	/*
	template
	<
		template <typename...> typename TemplateOut,
		typename... Ts
	>
	using instantiate_template_t = instantiate_template<TemplateOut, Ts...>::type;
	*/
}