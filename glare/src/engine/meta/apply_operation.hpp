#pragma once

#include "types.hpp"
#include "meta_value_operator.hpp"

//#include "meta.hpp"
#include "hash.hpp"
#include "indirection.hpp"
#include "function.hpp"

//#include <engine/entity/entity_target.hpp>

#include <engine/reflection/common_extensions.hpp>

#include <entt/meta/meta.hpp>
#include <entt/core/type_info.hpp>

#include <util/reflection.hpp>
#include <util/algorithm.hpp>
#include <util/type_algorithms.hpp>

#include <math/comparison.hpp>

#include <utility>
#include <type_traits>
#include <array>

namespace engine
{
	namespace impl
	{
		template <bool is_unary, typename Left, typename Right, typename ...Args>
		MetaAny invoke_operator_impl(const MetaFunction& operator_fn, Left&& left, Right&& right, Args&&... args)
		{
			constexpr std::size_t value_argument_count = ((is_unary) ? 1 : 2);

			if (operator_fn.is_static())
			{
				if (operator_fn.arity() >= (value_argument_count + sizeof...(Args)))
				{
					if constexpr (is_unary)
					{
						if (auto result = operator_fn.invoke({}, entt::forward_as_meta(left), entt::forward_as_meta<Args>(args)...))
						{
							return result;
						}
					}
					else
					{
						if (auto result = operator_fn.invoke({}, entt::forward_as_meta(left), entt::forward_as_meta(right), entt::forward_as_meta<Args>(args)...))
						{
							return result;
						}
					}

					if (const auto second_argument_type = operator_fn.arg(1))
					{
						const auto second_argument_type_id = second_argument_type.id();

						switch (second_argument_type_id)
						{
							case entt::type_hash<MetaAny>::value():
							case entt::type_hash<Registry>::value():
							case entt::type_hash<MetaEvaluationContext>::value():
								break;

							default:
								if (const auto from_type = right.type())
								{
									if (from_type.id() != entt::type_hash<MetaAny>::value())
									{
										// NOTE: Using simplified invocation syntax here could result in a copy of `right`.
										// For this reason, we manually construct the `arguments` array.
										if constexpr (is_unary)
										{
											std::array<MetaAny, (value_argument_count + sizeof...(Args))> arguments =
											{
												left.as_ref(),
												entt::forward_as_meta<Args>(args)...
											};

											if (auto result = operator_fn.invoke({}, arguments.data(), arguments.size()))
											{
												return result;
											}
										}
										else
										{
											std::array<MetaAny, (value_argument_count + sizeof...(Args))> arguments =
											{
												left.as_ref(),
												right.as_ref(),
												entt::forward_as_meta<Args>(args)...
											};

											if (auto result = operator_fn.invoke({}, arguments.data(), arguments.size()))
											{
												return result;
											}
										}
									}
								}
						}
					}
				}
			}
			else
			{
				if (operator_fn.arity() >= ((value_argument_count - 1) + sizeof...(Args)))
				{
					if constexpr (is_unary)
					{
						if (auto result = operator_fn.invoke(left, entt::forward_as_meta<Args>(args)...))
						{
							return result;
						}
					}
					else
					{
						if (auto result = operator_fn.invoke(left, entt::forward_as_meta(right), entt::forward_as_meta<Args>(args)...))
						{
							return result;
						}
					}

					if (const auto first_argument_type = operator_fn.arg(0))
					{
						const auto first_argument_type_id = first_argument_type.id();

						switch (first_argument_type_id)
						{
							case entt::type_hash<MetaAny>::value():
							case entt::type_hash<Registry>::value():
							case entt::type_hash<MetaEvaluationContext>::value():
								break;

							default:
								if (const auto from_type = right.type())
								{
									if (from_type.id() != entt::type_hash<MetaAny>::value())
									{
										// NOTE: Using simplified invocation syntax here could result in a copy of `right`.
										// For this reason, we manually construct the `arguments` array.
										if constexpr (is_unary)
										{
											std::array<MetaAny, (sizeof...(Args))> arguments =
											{
												entt::forward_as_meta<Args>(args)...
											};

											if (auto result = operator_fn.invoke(left, arguments.data(), arguments.size()))
											{
												return result;
											}
										}
										else
										{
											std::array<MetaAny, ((value_argument_count - 1) + sizeof...(Args))> arguments =
											{
												right.as_ref(),
												entt::forward_as_meta<Args>(args)...
											};

											if (auto result = operator_fn.invoke(left, arguments.data(), arguments.size()))
											{
												return result;
											}
										}
									}
								}
						}
					}
				}
			}

			return {};
		}

		template <typename OutputType, typename LeftType, typename RightType>
		OutputType apply_operation_exact_impl(const LeftType& left, const RightType& right, MetaValueOperator operation)
		{
			if constexpr (!std::is_same_v<std::decay_t<OutputType>, MetaAny>) // (std::is_arithmetic_v<OutputType>)
			{
				/*
				// TODO: Add specializations for an `OutputType` of `bool`.
				if constexpr (std::is_same_v<std::decay_t<OutputType>, bool>)
				{
					// ...
				}
				*/

				// TODO: Look into whether it makes sense to have
				// additional checks for operator availability.
				switch (operation)
				{
					case MetaValueOperator::UnaryPlus:
					case MetaValueOperator::UnaryMinus:
					case MetaValueOperator::LogicalNot:
					case MetaValueOperator::BitwiseNot:
					case MetaValueOperator::Boolean:
						return apply_unary_operation_exact<OutputType>(left, operation);

					case MetaValueOperator::Multiply:
						if constexpr (std::is_same_v<std::decay_t<LeftType>, bool>)
						{
							return ((left) && static_cast<bool>(right));
						}
						else
						{
							return (left * right);
						}

					case MetaValueOperator::Divide:
						if constexpr (std::is_same_v<std::decay_t<LeftType>, bool>)
						{
							return ((left) && static_cast<bool>(right));
						}
						else
						{
							return (left / right);
						}

					case MetaValueOperator::Modulus:
						if constexpr (std::is_integral_v<LeftType> && std::is_integral_v<RightType>)
						{
							if constexpr (!std::is_same_v<std::decay_t<LeftType>, bool>)
							{
								return (left % right);
							}
						}
						else
						{
							return std::fmod(static_cast<OutputType>(left), static_cast<OutputType>(right));
						}

						break;

					case MetaValueOperator::Add:
						return (left + right);

					case MetaValueOperator::Subtract:
						return (left - right);
				}

				if constexpr (std::is_integral_v<LeftType> && std::is_integral_v<RightType>)
				{
					if constexpr (std::is_same_v<std::decay_t<LeftType>, bool>)
					{
						switch (operation)
						{
							case MetaValueOperator::BitwiseAnd:
								return (left && static_cast<bool>(right));
							case MetaValueOperator::BitwiseOr:
								return (left || static_cast<bool>(right));
							case MetaValueOperator::BitwiseXOR:
								return (left != static_cast<bool>(right));
						}
					}
					else
					{
						switch (operation)
						{
							case MetaValueOperator::ShiftLeft:
								return (left << right);
							case MetaValueOperator::ShiftRight:
								return (left >> right);

							case MetaValueOperator::BitwiseAnd:
								return (left & right);
							case MetaValueOperator::BitwiseOr:
								return (left | right);
							case MetaValueOperator::BitwiseXOR:
								return (left ^ right);
						}
					}
				}
			}

			return {}; // static_cast<OutputType>(left);
		}
	}

	template <typename Left, typename Right, typename ...Args>
	MetaAny invoke_operator(MetaFunction operator_fn, Left&& left, Right&& right, Args&&... args)
	{
		using namespace engine::impl;

		// Exact / very similar arguments attempts
		// (Unwanted type conversions are avoided):
		if constexpr (sizeof...(args) > 0)
		{
			for (auto with_context = operator_fn; (with_context); with_context = with_context.next())
			{
				const auto function_arg_count = with_context.arity();

				std::size_t function_argument_offset = 0;

				if (with_context.is_static())
				{
					constexpr auto expected_static_function_arity = (2 + sizeof...(args));

					if (function_arg_count < expected_static_function_arity)
					{
						continue;
					}

					if (left)
					{
						const auto left_type = left.type();
						const auto first_arg_type = with_context.arg(0);

						if (!argument_has_invocation_priority(first_arg_type, left_type))
						{
							continue;
						}
					}

					if (right) // && (function_arg_count > 1)
					{
						const auto right_type = right.type();
						const auto second_arg_type = with_context.arg(1);

						if (!argument_has_invocation_priority(second_arg_type, right_type))
						{
							continue;
						}
					}

					// An offset of 2 to account for `left` and `right`.
					function_argument_offset = 2; // (expected_static_function_arity - sizeof...(args));
				}
				else
				{
					constexpr auto expected_dynamic_function_arity = (1 + sizeof...(args));

					if (function_arg_count < expected_dynamic_function_arity)
					{
						continue;
					}

					if (right) // && (function_arg_count > 1)
					{
						const auto right_type = right.type();
						const auto first_arg_type = with_context.arg(0);

						if (!argument_has_invocation_priority(first_arg_type, right_type))
						{
							continue;
						}
					}

					function_argument_offset = 1; // (expected_dynamic_function_arity - sizeof...(args));
				}

				assert(function_argument_offset);

				bool skip_overload = false;

				util::static_iota<sizeof...(args)>
				(
					[&]<std::size_t argument_index>()
					{
						using arg_t = util::get_type_by_index<argument_index, Args...>;

						if (!skip_overload)
						{
							const auto function_arg_type = with_context.arg(argument_index + function_argument_offset);
							const auto input_arg_type = resolve<arg_t>();

							if (!argument_has_invocation_priority(function_arg_type, input_arg_type))
							{
								skip_overload = true;
							}
						}
					}
				);

				if (skip_overload)
				{
					continue;
				}

				if (auto result = invoke_operator_impl<false>(with_context, left, right, args...))
				{
					return result;
				}
			}
		}

		for (auto without_context = operator_fn; (without_context); without_context = without_context.next())
		{
			const auto function_arg_count = without_context.arity();

			if (without_context.is_static())
			{
				constexpr auto expected_static_function_arity = 2;

				if (function_arg_count < expected_static_function_arity)
				{
					continue;
				}

				if (left)
				{
					const auto left_type = left.type();
					const auto first_arg_type = without_context.arg(0);

					if (!argument_has_invocation_priority(first_arg_type, left_type))
					{
						continue;
					}
				}

				if (right) // && (function_arg_count > 1)
				{
					const auto right_type = right.type();
					const auto second_arg_type = without_context.arg(1);

					if (!argument_has_invocation_priority(second_arg_type, right_type))
					{
						continue;
					}
				}
			}
			else
			{
				constexpr auto expected_dynamic_function_arity = 1;

				if (function_arg_count < expected_dynamic_function_arity)
				{
					continue;
				}

				if (right) // && (function_arg_count > 1)
				{
					const auto right_type = right.type();
					const auto first_arg_type = without_context.arg(0);

					if (!argument_has_invocation_priority(first_arg_type, right_type))
					{
						continue;
					}
				}
			}

			if (auto result = invoke_operator_impl<false>(without_context, left, right))
			{
				return result;
			}
		}

		// Fallback/regular invocation attempts
		// (Various type conversions may occur):
		if constexpr (sizeof...(args) > 0)
		{
			for (auto with_context = operator_fn; (with_context); with_context = with_context.next())
			{
				if (auto result = invoke_operator_impl<false>(with_context, left, right, args...))
				{
					return result;
				}
			}
		}

		for (auto without_context = operator_fn; (without_context); without_context = without_context.next())
		{
			if (auto result = invoke_operator_impl<false>(without_context, left, right))
			{
				return result;
			}
		}

		return {};
	}

	template <typename ValueType, typename ...Args>
	MetaAny invoke_unary_operator(MetaFunction operator_fn, ValueType&& value, Args&&... args)
	{
		using namespace engine::impl;

		if constexpr (sizeof...(args) > 0)
		{
			auto full_arguments = operator_fn;

			while (full_arguments)
			{
				if (auto result = invoke_operator_impl<true>(full_arguments, value, MetaAny {}, args...))
				{
					return result;
				}

				full_arguments = full_arguments.next();
			}
		}

		while (operator_fn)
		{
			if (auto result = invoke_operator_impl<true>(operator_fn, value, MetaAny {}))
			{
				return result;
			}

			operator_fn = operator_fn.next();
		}

		return {};
	}

	template <typename Callback, typename ReturnType=decltype(std::declval<Callback>()(std::declval<bool>(), std::declval<bool>()))> // bool
	ReturnType compare_boolean_values
	(
		bool left, bool right,
		Callback&& callback
	)
	{
		return callback(left, right);
	}

	template <typename Left, typename Right, typename Callback, typename ...Args>
	auto compare_boolean_values
	(
		Left&& left, Right&& right,
		bool require_existing_boolean,
		Callback&& callback,
		Args&&... args
	) -> decltype(std::declval<Callback>()(std::declval<bool>(), std::declval<bool>()))
	{
		using ReturnType = decltype(std::declval<Callback>()(std::declval<bool>(), std::declval<bool>()));

		auto as_bool_any = [&](auto&& value) -> MetaAny
		{
			if (auto bool_any = value.allow_cast<bool>())
			{
				return bool_any;
			}

			if (auto bool_any_from_operation = apply_operation(MetaAny {}, value, MetaValueOperator::Boolean, args...))
			{
				return bool_any_from_operation;
			}

			return {};
		};

		if (left && right)
		{
			const auto left_type = left.type();
			const auto right_type = right.type();

			if (left_type.id() == entt::type_hash<bool>::value()) // (left_type == resolve<bool>())
			{
				const auto left_as_bool = left.try_cast<bool>();

				// Use both `left` and `right` as-is.
				if (right_type.id() == entt::type_hash<bool>::value()) // (right_type == resolve<bool>())
				{
					const auto right_as_bool = right.try_cast<bool>();

					return callback(*left_as_bool, *right_as_bool);
				}
				// Convert `right` to `bool`, use `left` as-is.
				else if (auto right_as_bool_any = as_bool_any(right))
				{
					return callback(*left_as_bool, right_as_bool_any.cast<bool>());
				}
			}
			else if (right_type.id() == entt::type_hash<bool>::value()) // (right_type == resolve<bool>())
			{
				// Convert `left` to `bool`, use `right` as-is.
				if (auto left_as_bool_any = as_bool_any(left))
				{
					const auto right_as_bool = right.try_cast<bool>();

					return callback(left_as_bool_any.cast<bool>(), *right_as_bool);
				}
			}
			else if (!require_existing_boolean)
			{
				// Convert both `left` and `right` to `bool`.
				if (auto left_as_bool_any = as_bool_any(left))
				{
					if (auto right_as_bool_any = as_bool_any(right))
					{
						return callback(left_as_bool_any.cast<bool>(), right_as_bool_any.cast<bool>());
					}
				}
			}
		}

		if (!std::is_void_v<ReturnType>)
		{
			return {};
		}
	}

	template <typename Left, typename Right, typename ...Args>
	MetaAny apply_operation(Left&& left, Right&& right, MetaValueOperator operation, Args&&... args)
	{
		using namespace engine::literals;

		if (right)
		{
			if (left)
			{
				if (operation == MetaValueOperator::Dereference)
				{
					operation = MetaValueOperator::Get;
				}
			}
			else
			{
				if ((operation != MetaValueOperator::Get) && (!is_unary_operation(operation)))
				{
					return {};
				}
			}
		}
		else
		{
			return {};
		}

		auto left_type = left.type();
		auto right_type = right.type();

		bool use_advanced_get_logic = false;

		if (operation == MetaValueOperator::Get)
		{
			if (left)
			{
				// Explicit checks for retrieval from a `null` entity value:
				if (right_type.id() == "EntityTarget"_hs) // (right_type == resolve<EntityTarget>())
				{
					if (auto right_as_entity_any = try_get_underlying_value(right, args...))
					{
						if (auto right_as_entity = right_as_entity_any.try_cast<Entity>())
						{
							if ((*right_as_entity) == null)
							{
								// Retrieval from `null` is considered invalid.
								return {};
							}
						}
					}
				}
				else if (auto right_as_entity = right.try_cast<Entity>())
				{
					if ((*right_as_entity) == null)
					{
						// Retrieval from `null` is considered invalid.
						return {};
					}
				}
			}

			/*
			if constexpr (sizeof...(args) == 0)
			{
				use_advanced_get_logic = true;
			}
			else if constexpr (std::is_same_v<std::decay_t<util::get_first_type<Args...>>, MetaAny>)
			{
				use_advanced_get_logic = true;
			}
			*/

			use_advanced_get_logic = true;
		}

		if (use_advanced_get_logic) // && (sizeof...(args) > 0)
		{
			auto right_resolved = MetaAny {};

			if (type_has_indirection(left_type))
			{
				right_resolved = try_get_underlying_value(static_cast<const MetaAny&>(right), args...);

				if (right_resolved)
				{
					if (value_has_indirection(right_resolved))
					{
						// NOTE: Recursion.
						return apply_operation(left, right_resolved, operation, args...);
					}
					else
					{
						if (auto result = try_get_underlying_value(left, right_resolved, args...))
						{
							return result;
						}
					}
				}
			}

			if (type_has_indirection(right_type))
			{
				if (right_resolved)
				{
					if (auto left_with_resolved_right = try_get_underlying_value(static_cast<const MetaAny&>(left), right_resolved, args...))
					{
						return left_with_resolved_right;
					}
				}

				if (auto left_resolved = try_get_underlying_value(static_cast<const MetaAny&>(left), args...))
				{
					if (auto result = try_get_underlying_value(left_resolved, right, args...))
					{
						return result;
					}

					if (right_resolved)
					{
						if (auto result = try_get_underlying_value(left_resolved, right_resolved, args...))
						{
							return result;
						}
					}
				}
			}
		}

		const bool operation_is_assignment = is_assignment_operation(operation);

		// If `left` has value indirection, resolve it:
		if (!operation_is_assignment)
		{
			if (auto left_result = try_get_underlying_value(static_cast<const MetaAny&>(left), args...))
			{
				return apply_operation(left_result, right, operation, std::forward<Args>(args)...);
			}
		}

		// If `right` has value indirection, resolve it:
		if (auto right_result = try_get_underlying_value(static_cast<const MetaAny&>(right), args...))
		{
			return apply_operation(left, right_result, operation, std::forward<Args>(args)...);
		}

		if (operation_is_assignment)
		{
			constexpr bool is_chain_operation = (sizeof...(args) > 3);

			if constexpr (is_chain_operation)
			{
				// Drop the first argument (`MetaAny` used as a 'source'),
				// then reduce the number of arguments until
				// something (hopefully) works for reducing `right`.
				// 
				// If all else fails, continue anyway:
				auto right_result = util::drop_first
				(
					[&right](auto&&... reduced_args)
					{
						return util::drop_last_until_success<2>
						(
							[&right](auto&&... reduced_args)
							{
								return try_get_underlying_value(right, reduced_args...);
							},

							reduced_args...
						);
					},

					args...
				);

				if (right_result)
				{
					return apply_operation(left, right_result, operation, std::forward<Args>(args)...); // args...
				}
			}
		}
		else
		{
			if (right_type)
			{
				if (right_type.is_arithmetic())
				{
					if (!left && right)
					{
						// Unary operator control-path.
						return apply_arithmetic_operation
						(
							right_type,
							right,
							operation
						);
					}
					else if (left_type)
					{
						if ((right_type == resolve<bool>()) && (!type_has_indirection(left_type))) // (right_type.id() == "bool"_hs)
						{
							// Boolean comparison.
							auto boolean_result = apply_arithmetic_operation
							(
								right_type,
								left, right,
								operation
							);

							if (boolean_result)
							{
								return boolean_result;
							}
						}
					}
				}

				if
				(
					(left && left_type && left_type.is_arithmetic())
					&&
					(
						(right_type.is_arithmetic())
						||
						((left_type == resolve<bool>()) && (!type_has_indirection(right_type)))
					)
				)
				{
					// Standard arithmetic-type operator implementations.
					auto arithmetic_result = apply_arithmetic_operation
					(
						left_type,

						left, right,
						operation
					);

					if (arithmetic_result)
					{
						return arithmetic_result;
					}
				}
			}
		}

		// Standard operator dispatch:
		const auto operator_name = get_operator_name(operation);
			
		if (auto operator_fn = left_type.func(operator_name))
		{
			if (auto result = invoke_operator(operator_fn, left, right, args...))
			{
				return result;
			}
		}

		if (auto operator_fn = right_type.func(operator_name))
		{
			if (left)
			{
				if (auto result = invoke_operator(operator_fn, left, right, args...))
				{
					return result;
				}
			}
			else
			{
				if (auto result = invoke_unary_operator(operator_fn, right, args...))
				{
					return result;
				}
			}
		}

		if (operation_is_assignment && (operation != MetaValueOperator::Assign))
		{
			if (const auto underlying_operation = decay_operation(operation); (underlying_operation != operation))
			{
				if (auto value_result = apply_operation(left, right, underlying_operation, args...))
				{
					if (auto assignment_result = apply_operation(left, value_result, MetaValueOperator::Assign, args...))
					{
						return assignment_result;
					}
				}
			}
		}

		// Common operator fallbacks:
		switch (operation)
		{
			case MetaValueOperator::Dereference:
				if (auto dereferenced = *right)
				{
					return dereferenced;
				}

				if (right_type.is_sequence_container())
				{
					return right.as_sequence_container();
				}
				else if (right_type.is_associative_container())
				{
					return right.as_associative_container();
				}

				break;

			case MetaValueOperator::Subscript:
				if (left_type.is_sequence_container())
				{
					if (auto as_container = left.as_sequence_container())
					{
						if (auto it = right.try_cast<entt::meta_sequence_container::iterator>()) // (right_type.id() == entt::type_hash<entt::meta_sequence_container::iterator>::value())
						{
							// NOTE: There's no check here for relationship between iterator and source container.
							// (Not currently supported by EnTT's API)
							return *(*it);
						}
						else if (auto right_as_index_any = std::as_const(right).allow_cast<std::size_t>())
						{
							if (auto right_as_index = right_as_index_any.try_cast<std::size_t>())
							{
								if (auto result = as_container[*right_as_index])
								{
									return result;
								}
							}
						}
					}
				}
				else if (left_type.is_associative_container())
				{
					if (auto as_container = left.as_associative_container())
					{
						const auto key_type = as_container.key_type();

						if (right_type == key_type)
						{
							if (auto it = as_container.find(right); it != as_container.end())
							{
								if (auto& result = it->second)
								{
									return result; // .as_ref();
								}
							}
						}
						else
						{
							if (auto right_as_key_any = std::as_const(right).allow_cast(key_type))
							{
								if (auto it = as_container.find(right_as_key_any); it != as_container.end())
								{
									if (auto& result = it->second)
									{
										return result; // .as_ref();
									}
								}
							}
						}
					}
				}

				break;

			// NOTE: For most scenarios we could probably fall through on these two
			// operations, rather than using `MetaValueOperator::Equal` explicitly.
			// 
			// However, since there could be a custom `operator==` implementation
			// that EnTT doesn't identify, we'll recurse anyway.

			case MetaValueOperator::GreaterThanOrEqual:
				if (auto result = apply_operation(left, right, MetaValueOperator::GreaterThan, args...))
				{
					if (const auto as_bool = result.try_cast<bool>())
					{
						if (*as_bool)
						{
							return result;
						}
					}
				}

				return apply_operation(left, right, MetaValueOperator::Equal, args...);

			case MetaValueOperator::LessThanOrEqual:
				if (auto result = apply_operation(left, right, MetaValueOperator::LessThan, args...))
				{
					if (const auto as_bool = result.try_cast<bool>())
					{
						if (*as_bool)
						{
							return result;
						}
					}
				}

				return apply_operation(left, right, MetaValueOperator::Equal, args...);

			case MetaValueOperator::Equal:
				if (compare_boolean_values(left, right, true, [](bool left_as_bool, bool right_as_bool) { return (left_as_bool == right_as_bool); }, args...))
				{
					return true;
				}
				else
				{
					// Special behavior for `null` value in equality comparison:
					// TODO: Look into implementing 'symbolic-null' as an alternative, rather than using `Entity`'s `null` value.

					if (auto left_as_null = left.try_cast<Entity>(); ((left_as_null) && ((*left_as_null) == null)))
					{
						if (auto right_as_entity = right.try_cast<Entity>())
						{
							return (null == (*right_as_entity)); // ((*left_as_null) == ...);
						}
						else if (value_has_indirection(right))
						{
							// Indirection is considered 'null-equivalent' in a comparison.
							// (i.e. Any indirection that has made it this far is due to a failure to resolve)
							return true;
						}
						else
						{
							return (!static_cast<bool>(right));
						}
					}

					if (auto right_as_null = right.try_cast<Entity>(); ((right_as_null) && ((*right_as_null) == null)))
					{
						if (auto left_as_entity = left.try_cast<Entity>())
						{
							return ((*left_as_entity) == null); // (... == (*right_as_null));
						}
						else if (value_has_indirection(left))
						{
							// Indirection is considered 'null-equivalent' in a comparison.
							// (i.e. Any indirection that has made it this far is due to a failure to resolve)
							return true;
						}
						else
						{
							return (!static_cast<bool>(left));
						}
					}

					return (left == right);
				}

				break;

			case MetaValueOperator::NotEqual:
				if (compare_boolean_values(left, right, true, [](bool left_as_bool, bool right_as_bool) { return (left_as_bool != right_as_bool); }, args...))
				{
					return true;
				}
				else
				{
					// Special behavior for `null` value in equality comparison:
					// TODO: Look into implementing 'symbolic-null' as an alternative, rather than using `Entity`'s `null` value.

					if (auto left_as_null = left.try_cast<Entity>(); ((left_as_null) && ((*left_as_null) == null)))
					{
						if (auto right_as_entity = right.try_cast<Entity>())
						{
							return (null != (*right_as_entity)); // ((*left_as_null) != ...);
						}
						else if (value_has_indirection(right))
						{
							// Indirection is considered 'null-equivalent' in a comparison.
							// (i.e. Any indirection that has made it this far is due to a failure to resolve)
							return false;
						}
						else
						{
							return (static_cast<bool>(right));
						}
					}

					if (auto right_as_null = right.try_cast<Entity>(); ((right_as_null) && ((*right_as_null) == null)))
					{
						if (auto left_as_entity = left.try_cast<Entity>())
						{
							return ((*left_as_entity) != null); // (... != (*right_as_null));
						}
						else if (value_has_indirection(left))
						{
							// Indirection is considered 'null-equivalent' in a comparison.
							// (i.e. Any indirection that has made it this far is due to a failure to resolve)
							return false;
						}
						else
						{
							return (static_cast<bool>(left));
						}
					}

					return (left != right);
				}

				break;
		}

		// Arithmetic conversion fallbacks:
		if (left_type.is_arithmetic())
		{
			if (auto right_out = right.allow_cast(left_type))
			{
				if (auto result = apply_arithmetic_operation(left_type, left, right_out, operation))
				{
					return result;
				}
			}
		}

		if (right_type.is_arithmetic())
		{
			if (auto left_out = left.allow_cast(right_type))
			{
				if (auto result = apply_arithmetic_operation(right_type, left_out, right, operation))
				{
					return result;
				}
			}
		}

		//print_warn("Unable to resolve operator function ({}) for types #{} & #{}.", operation, left_type.id(), right_type.id());

		// Unary `Get` operation.
		if ((operation == MetaValueOperator::Get) && (!left))
		{
			return right;
		}

		return {};
	}

	template <typename OutputType, typename LeftType>
	OutputType apply_unary_operation_exact(const LeftType& left, MetaValueOperator operation)
	{
		switch (operation)
		{
			case MetaValueOperator::UnaryPlus:
				//return +left; // std::abs(left);

				return impl::operator_unary_plus_impl(left);

			case MetaValueOperator::UnaryMinus:
				//return -left;

				return impl::operator_unary_minus_impl(left);

			case MetaValueOperator::BitwiseNot:
				if constexpr (std::is_integral_v<LeftType>)
				{
					if constexpr (std::is_same_v<std::decay_t<LeftType>, bool>)
					{
						return !left;
					}
					else
					{
						return ~left;
					}
				}

				break;

			case MetaValueOperator::Boolean:
				// Do nothing; just cast to `OutputType`.
				// Boolean implementation can be found in `apply_operation_exact` wrapper function.
				break;

			case MetaValueOperator::LogicalNot:
				// Do nothing; just cast to `OutputType`.
				// Boolean implementation can be found in `apply_operation_exact` wrapper function.
				break;
		}

		return static_cast<OutputType>(left);
	}

	template <typename OutputType, typename LeftType, typename RightType>
	MetaAny apply_operation_exact(const LeftType& left, const RightType& right, MetaValueOperator operation)
	{
		using namespace engine::impl;

		constexpr bool is_arithmetic = (std::is_arithmetic_v<LeftType> || std::is_arithmetic_v<RightType>);

		if constexpr (is_arithmetic)
		{
			if constexpr (std::is_integral_v<OutputType>)
			{
				switch (operation)
				{
					case MetaValueOperator::Equal:
						return (static_cast<OutputType>(left) == static_cast<OutputType>(right));
					case MetaValueOperator::NotEqual:
						return (static_cast<OutputType>(left) != static_cast<OutputType>(right));
				}
			}
			else // if constexpr (std::is_floating_point_v<OutputType>)
			{
				switch (operation)
				{
					case MetaValueOperator::Equal:
						return math::fcmp(static_cast<OutputType>(left), static_cast<OutputType>(right));
				
					case MetaValueOperator::NotEqual:
						return !math::fcmp(static_cast<OutputType>(left), static_cast<OutputType>(right));

					//case MetaValueOperator::LessThanOrEqual:
						//return ((static_cast<OutputType>(left) < static_cast<OutputType>(right)) || math::fcmp(static_cast<OutputType>(left), static_cast<OutputType>(right)));
					//case MetaValueOperator::GreaterThanOrEqual:
						//return ((static_cast<OutputType>(left) > static_cast<OutputType>(right)) || math::fcmp(static_cast<OutputType>(left), static_cast<OutputType>(right)));
				}
			}
			
			switch (operation)
			{
				case MetaValueOperator::LessThan:
					return (static_cast<OutputType>(left) < static_cast<OutputType>(right));
				case MetaValueOperator::GreaterThan:
					return (static_cast<OutputType>(left) > static_cast<OutputType>(right));
				case MetaValueOperator::LessThanOrEqual:
					return (static_cast<OutputType>(left) <= static_cast<OutputType>(right));
				case MetaValueOperator::GreaterThanOrEqual:
					return (static_cast<OutputType>(left) >= static_cast<OutputType>(right));
			}
		}

		auto result = apply_operation_exact_impl<OutputType, LeftType, RightType>(left, right, operation);

		auto to_boolean = [&result]() -> bool
		{
			/*
			constexpr bool is_meta_any =
			(
				std::is_same_v<std::decay_t<OutputType>, MetaAny>
				//||
				//std::is_same_v<std::decay_t<LeftType>, MetaAny>
				//||
				//std::is_same_v<std::decay_t<RightType>, MetaAny>
			);
			*/

			constexpr bool is_meta_any = false;

			if constexpr (is_meta_any)
			{
				if (result)
				{
					if (auto* computed_value = result.try_cast<OutputType>())
					{
						//return static_cast<bool>(*computed_value);
						return impl::operator_bool_impl(*computed_value);
					}
				}
				else
				{
					return false;
				}

				//return false;
				return static_cast<bool>(result);
			}
			else
			{
				if constexpr (std::is_constructible_v<bool, decltype((std::declval<const OutputType&>()))>)
				{
					//return static_cast<bool>(result);
					return impl::operator_bool_impl(result);
				}
				else
				{
					return true; // false;
				}
			}
		};

		switch (operation)
		{
			case MetaValueOperator::Boolean:
				return to_boolean();

			case MetaValueOperator::LogicalNot:
				return !to_boolean();
		}

		return result;
	}

	template <typename OutputType>
	MetaAny apply_operation_exact(const MetaAny& left, const MetaAny& right, MetaValueOperator operation)
	{
		auto left_out = OutputType {};

		if (left)
		{
			if (auto left_casted = left.allow_cast<OutputType>())
			{
				left_out = left_casted.cast<OutputType>();
			}
			else
			{
				if constexpr (std::is_same_v<std::decay_t<OutputType>, bool>)
				{
					left_out = true;
				}
			}
		}

		auto right_out = OutputType {};
		
		if (right)
		{
			if (auto right_casted = right.allow_cast<OutputType>())
			{
				right_out = right_casted.cast<OutputType>();
			}
			else
			{
				if constexpr (std::is_same_v<std::decay_t<OutputType>, bool>)
				{
					right_out = true;
				}
			}
		}

		return apply_operation_exact<OutputType, OutputType, OutputType>(left_out, right_out, operation);
	}

	inline MetaAny apply_arithmetic_operation(const MetaType& output_type, const MetaAny& left, const MetaAny& right, MetaValueOperator operation)
	{
		if (!output_type.is_arithmetic())
		{
			return {};
		}

		if (output_type.is_integral())
		{
			switch (output_type.id())
			{
				case entt::type_hash<bool>::value():
					return apply_operation_exact<bool>(left, right, operation);

				case entt::type_hash<std::int64_t>::value():
					return apply_operation_exact<std::int64_t>(left, right, operation);

				case entt::type_hash<std::uint64_t>::value():
					return apply_operation_exact<std::uint64_t>(left, right, operation);
				
				// Promote smaller integral types to 32-bit:
				default:
					if (output_type.is_signed())
					{
						return apply_operation_exact<std::int32_t>(left, right, operation);
					}
					else
					{
						return apply_operation_exact<std::uint32_t>(left, right, operation);
					}

					break;
			}
		}
		else
		{
			switch (output_type.id())
			{
				case entt::type_hash<double>::value():
					return apply_operation_exact<double>(left, right, operation);

				case entt::type_hash<long double>::value():
					return apply_operation_exact<long double>(left, right, operation);

				case entt::type_hash<float>::value():
					return apply_operation_exact<float>(left, right, operation);
			}
		}

		return {};
	}

	inline MetaAny apply_arithmetic_operation(const MetaType& output_type, const MetaAny& value, MetaValueOperator operation)
	{
		//assert(is_unary_operation(operation));

		return apply_arithmetic_operation(output_type, value, {}, operation);
	}
}