#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

namespace util
{
	namespace impl
	{
		template <typename T, typename TupleType>
		struct tuple_contains_impl;

		template <typename T>
		struct tuple_contains_impl<T, std::tuple<>>
			: std::false_type {};

		template <typename T, typename U, typename ...Ts>
		struct tuple_contains_impl<T, std::tuple<U, Ts...>>
			: tuple_contains_impl<T, std::tuple<Ts...>> {};

		template <typename T, typename ...Ts>
		struct tuple_contains_impl<T, std::tuple<T, Ts...>>
			: std::true_type {};
	}

	template <typename TupleType, typename T>
	using tuple_contains = impl::tuple_contains_impl<T, TupleType>;

	template <typename TupleType, typename T>
	inline constexpr bool tuple_contains_v = tuple_contains<TupleType, T>::value;

	template <typename TupleType, typename ...Ts>
	using tuple_contains_any = std::disjunction<tuple_contains<TupleType, Ts>...>;

	template <typename TupleType, typename ...Ts>
	inline constexpr bool tuple_contains_any_v = tuple_contains_any<TupleType, Ts...>::value;

	template <typename TupleType, typename ...Ts>
	using tuple_contains_all = std::conjunction<tuple_contains<TupleType, Ts>...>;

	template <typename TupleType, typename ...Ts>
	inline constexpr bool tuple_contains_all_v = tuple_contains_all<TupleType, Ts...>::value;

	static_assert(tuple_contains_any_v<std::tuple<int, short>, char, int>);
	static_assert(!tuple_contains_any_v<std::tuple<int, float, short>, char, double>);

	static_assert(tuple_contains_all_v<std::tuple<int, short>, int>);
	static_assert(tuple_contains_all_v<std::tuple<int, short>, int, short>);
	static_assert(!tuple_contains_all_v<std::tuple<short>, char, short>);
	static_assert(!tuple_contains_all_v<std::tuple<short, int>, char, short>);

	static_assert(tuple_contains_v<std::tuple<int, float, short>, int>);
	static_assert(tuple_contains_v<std::tuple<int, float, short>, float>);
	static_assert(tuple_contains_v<std::tuple<int, float, short>, short>);
	static_assert(!tuple_contains_v<std::tuple<int, float, short>, double>);

	template <typename ...TupleTypes>
	using concat_tuple_types = decltype(std::tuple_cat(std::declval<TupleTypes>()...));

	template <typename T, typename ...TupleTypes>
	using tuple_push_front = concat_tuple_types<std::tuple<T>, TupleTypes...>;

	template <typename T, typename ...TupleTypes>
	using tuple_push_back = concat_tuple_types<TupleTypes..., std::tuple<T>>;

	template <typename T, typename ...TupleTypes>
	using tuple_add = tuple_push_back<T, TupleTypes...>;

	static_assert(std::is_same_v<concat_tuple_types<std::tuple<int>, std::tuple<float, short>>, std::tuple<int, float, short>>);
	static_assert(std::is_same_v<tuple_push_back<short, std::tuple<int>>, std::tuple<int, short>>);
	static_assert(std::is_same_v<tuple_push_front<short, std::tuple<int>>, std::tuple<short, int>>);

	namespace impl
	{
		template
		<
			template <typename...> typename TemplateOut,
			typename tuple_type,
			std::size_t offset,
			typename Sequence
		>
		struct instantiate_template_tuple_impl;

		template
		<
			template <typename...> typename TemplateOut,
			typename tuple_type,
			std::size_t offset,
			std::size_t... I
		>
		struct instantiate_template_tuple_impl
		<
			TemplateOut,
			tuple_type,
			offset,
			std::index_sequence<I...>
		>
		{
			using type = TemplateOut<typename std::tuple_element<(I + offset), tuple_type>::type...>;
		};

		template <typename TupleType, typename GenericFunctor, typename Indices>
		struct deplete_tuple_types_impl;

		template <typename TupleType, typename GenericFunctor, std::size_t... I>
		struct deplete_tuple_types_impl<TupleType, GenericFunctor, std::index_sequence<I...>>
		{
			template <std::size_t min_args=0>
			static constexpr void execute(GenericFunctor&& functor_instance)
			{
				functor_instance.template operator()<typename std::tuple_element<I, TupleType>::type...>();

				if constexpr (sizeof...(I) > min_args)
				{
					deplete_tuple_types_impl
					<
						TupleType, GenericFunctor,
						std::make_index_sequence<(sizeof...(I)-1)>
					>::template execute<min_args>(std::forward<decltype(functor_instance)>(functor_instance));
				}
			}

			template <std::size_t min_args=0>
			static constexpr void execute()
			{
				execute<min_args>(GenericFunctor());
			}
		};
	}

	/*
		Attempts to instantiate `TemplateOut` using the types held in the `tuple_type` specified.
		
		The `offset` and `n_truncated` parameters may be used to select which
		of the types in `tuple_type` are forwarded to `TemplateOut`.
	*/
	template
	<
		template <typename...> typename TemplateOut,
		typename tuple_type,
		std::size_t offset=0,
		std::size_t n_truncated=0
	>
	struct instantiate_template_with_tuple : impl::instantiate_template_tuple_impl
	<
		TemplateOut,
		tuple_type,
		offset,
		std::make_index_sequence<(std::tuple_size<tuple_type>::value - offset - n_truncated)>
	> {};

	/*
	template
	<
		template <typename...> typename TemplateOut,
		typename tuple_type,
		std::size_t offset=0,
		std::size_t n_truncated=0
	>
	using instantiate_template_with_tuple_t = instantiate_template_with_tuple
	<
		TemplateOut,
		tuple_type,
		offset,
		n_truncated
	>::type;
	*/

	// Calls an instance of `stateless_generic_lambda_t` with the types in `TupleType`,
	// removing types from the end of the sequence until `min_args` types are left.
	template <typename stateless_generic_lambda_t, typename TupleType, std::size_t min_args=0>
	constexpr void deplete_tuple_types_ex()
	{
		using impl_type = impl::deplete_tuple_types_impl
		<
			TupleType,
			stateless_generic_lambda_t,
			std::make_index_sequence<std::tuple_size<TupleType>::value>
		>;

		impl_type::template execute<min_args>();
	}

	// Calls `stateless_generic_lambda` with the types in `TupleType`,
	// removing types from the end of the sequence until `min_args` types are left.
	template <auto stateless_generic_lambda, typename TupleType, std::size_t min_args=0>
	constexpr void deplete_tuple_types()
	{
		deplete_tuple_types<decltype(stateless_generic_lambda), TupleType, min_args>();
	}

	// Calls `lambda_instance` with the types in `TupleType`,
	// removing types from the end of the sequence until `min_args` types are left.
	template <typename TupleType, std::size_t min_args, typename GenericLambda>
	constexpr void deplete_tuple_types(GenericLambda&& lambda_instance)
	{
		using impl_type = impl::deplete_tuple_types_impl
		<
			TupleType,
			std::remove_reference_t<decltype(lambda_instance)>,
			std::make_index_sequence<std::tuple_size<TupleType>::value>
		>;

		impl_type::template execute<min_args>(std::forward<GenericLambda>(lambda_instance));
	}
}