#pragma once

#include "types.hpp"
#include "indirection.hpp"

#include <util/small_vector.hpp>
#include <util/type_algorithms.hpp>

#include <type_traits>

#include <vector>

namespace engine
{
	// Returns true if the specific overload pointed to by `function` has a parameter of type `MetaAny`.
	bool function_overload_has_meta_any_argument(const MetaFunction& function);

	/*
		If `check_overloads` is true, this will return whether any of the overloads of `function` has a parameter of type `MetaAny`.
		
		If `check_overloads` is false, this will only check the first overload specified by `function`.
		(Equivalent to `function_overload_has_meta_any_argument`)
	*/
	bool function_has_meta_any_argument(const MetaFunction& function, bool check_overloads=true);

	// Returns true if any of the overloads referenced by `function_overloads` reports true for `MetaFunction::is_static`.
	bool function_has_static_overload(const MetaFunction& function_overloads);

	// Returns true if any of the overloads referenced by `function_overloads` reports true for `MetaFunction::is_const`.
	bool function_has_const_overload(const MetaFunction& function_overloads);

	// Returns true if `input_arg_type` meets the prioritization criteria for `function_arg_type`.
	// If `exact_match` is enabled, this will only check if the two types are equal.
	bool argument_has_invocation_priority(const MetaType& function_arg_type, const MetaType& input_arg_type, bool exact_match=false);

	/*
		Attempts to invoke `function` with the `args` specified.
		
		If invocation fails, the next overload of `function` will be attempted.
		If all overloads fail, this function will return an empty `MetaAny` object.
		
		NOTE: This uses `entt::forward_as_meta` automatically when passing `args` to the active function's `invoke` method.
	*/
	template <typename InstanceType, typename ...Args>
	MetaAny invoke_any_overload(const MetaFunction& first_overload, InstanceType&& instance, Args&&... args)
	{
		static_assert(std::is_same_v<std::decay_t<InstanceType>, MetaAny>, "The `instance` parameter must be a `MetaAny` object, or a reference to one.");

		// Try exact / very similar argument matches first:
		for (auto function = first_overload; (function); function = function.next())
		{
			// NOTE: Function `arity` check added to avoid unnecessary overhead from `entt::forward_as_meta`.
			if (function.arity() == sizeof...(args)) // >=
			{
				bool skip_overload = false;

				util::static_iota<sizeof...(args)>
				(
					[&]<std::size_t argument_index>()
					{
						using arg_t = util::get_type_by_index<argument_index, Args...>;

						if (!skip_overload)
						{
							const auto function_arg_type = function.arg(argument_index);
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

				if (auto result = function.invoke(instance, entt::forward_as_meta(args)...))
				{
					return result;
				}
			}
		}

		// Fallback to trying every overload, regardless of possible conversions:
		for (auto function = first_overload; (function); function = function.next())
		{
			// NOTE: Function `arity` check added to avoid unnecessary overhead from `entt::forward_as_meta`.
			if (function.arity() == sizeof...(args)) // >=
			{
				if (auto result = function.invoke(instance, entt::forward_as_meta(args)...))
				{
					return result;
				}
			}
		}

		return {};
	}

	/*
		Attempts to execute each function in `function_overloads` using `instance` and `arg_count` arguments from `args`.
		
		If `attempt_default_argument_expansion` is enabled, this will attempt to invoke the functions in
		`function_overloads` with defaulted values in place of potentially missing arguments.
	*/
	template <typename InstanceType> // typename ...Args
	MetaAny invoke_any_overload(MetaFunction function_overloads, InstanceType&& instance, MetaAny* const args, std::size_t arg_count, bool attempt_default_argument_expansion=false)
	{
		static_assert(std::is_same_v<std::decay_t<InstanceType>, MetaAny>, "The `instance` parameter must be a `MetaAny` object, or a reference to one.");

		// Try exact / very similar argument matches first:
		for (auto function = function_overloads; (function); function = function.next())
		{
            if (arg_count == function.arity()) // >=
            {
				bool skip_overload = false;

				for (std::size_t argument_index = 0; argument_index < arg_count; argument_index++) // function.arity()
				{
					const auto function_arg_type = function.arg(argument_index);
					const auto input_arg_type = args[argument_index].type();

					if (!argument_has_invocation_priority(function_arg_type, input_arg_type))
					{
						skip_overload = true;

						break;
					}
				}

				if (skip_overload)
				{
					continue;
				}

			    if (auto result = function.invoke(instance, args, arg_count))
			    {
				    return result;
			    }
            }
		}

		// Fallback to trying every overload, regardless of possible conversions:
		for (auto function = function_overloads; (function); function = function.next())
		{
			// NOTE: We don't bother with the function `arity` check here,
			// since EnTT already handles that. (+ Copying a pointer is essentially free)
            //if (arg_count == function.arity()) // >=
            {
			    if (auto result = function.invoke(instance, args, arg_count))
			    {
				    return result;
			    }
            }
		}

		if (attempt_default_argument_expansion)
		{
			using TemporaryArgumentStore = util::small_vector<MetaAny, 8>;

			TemporaryArgumentStore forwarding_store;

			forwarding_store.reserve(arg_count);

			for (std::size_t provided_argument_index = 0; provided_argument_index < arg_count; provided_argument_index++)
			{
				forwarding_store.emplace_back(args[provided_argument_index].as_ref());
			}

			for (auto function = function_overloads; (function); function = function.next())
			{
				const auto intended_arg_count = function.arity();

				if (arg_count < intended_arg_count)
				{
					bool attempt_defaulted_call = true;

					for (std::size_t defaulted_argument_index = arg_count; defaulted_argument_index < intended_arg_count; defaulted_argument_index++)
					{
						auto arg_type = function.arg(defaulted_argument_index);

						if (auto defaulted_argument = arg_type.construct())
						{
							forwarding_store.emplace_back(std::move(defaulted_argument));
						}
						else
						{
							attempt_defaulted_call = false;

							break;
						}
					}

					if (attempt_defaulted_call)
					{
						// NOTE: Const-cast needed due to limitations of EnTT's API.
						if (auto result = function.invoke(instance, const_cast<MetaAny* const>(forwarding_store.data()), forwarding_store.size()))
						{
							return result;
						}
					}

					forwarding_store.resize(arg_count);
				}
			}
		}

		return {};
	}

	/*
		This function attempts to invoke an overload of `function` that takes `arg_count` from `args`.
		
		If this fails, a second invocation pass will be attempted where any overload's parameters
		of type `MetaAny` will be handled via an `entt::forward_as_meta` passthrough.
	*/
    template <typename InstanceType>
	MetaAny invoke_any_overload_with_automatic_meta_forwarding(MetaFunction function_overloads, InstanceType&& instance, MetaAny* const args, std::size_t arg_count)
	{
		static_assert(std::is_same_v<std::decay_t<InstanceType>, MetaAny>, "The `instance` parameter must be a `MetaAny` object, or a reference to one.");

		if (auto result = invoke_any_overload(function_overloads, instance, args, arg_count))
		{
			return result;
		}

		using TemporaryArgumentStore = util::small_vector<MetaAny, 8>;

		TemporaryArgumentStore forwarding_store;

		for (auto function = function_overloads; (function); function = function.next())
		{
			const auto intended_arg_count = function.arity();

			if ((arg_count == intended_arg_count) && function_overload_has_meta_any_argument(function)) // >=
			{
				forwarding_store.reserve(intended_arg_count); // arg_count

				for (std::size_t i = 0; i < intended_arg_count; i++)
				{
					auto arg_type = function.arg(i);

					if (arg_type.id() == entt::type_hash<MetaAny>::value()) // resolve<MetaAny>().id()
					{
						forwarding_store.emplace_back(entt::forward_as_meta(args[i].as_ref()));
					}
					else
					{
						forwarding_store.emplace_back(args[i].as_ref());
					}
				}

				// NOTE: Const-cast needed due to limitations of EnTT's API.
				if (auto result = function.invoke(instance, const_cast<MetaAny* const>(forwarding_store.data()), forwarding_store.size()))
				{
					return result;
				}

				forwarding_store.clear();
			}
		}

		return {};
	}

	/*
		Attempts to execute each overload of `function` using `self` and `function_arguments`.
		The `function_arguments` parameter is assumed to be a vector-like type. (e.g. `util::small_vector`)
		
		If `handle_indirection` is enabled, each argument in `function_arguments` will be 'resolved' prior to invocation (if possible).
		
		If `attempt_to_forward_context` is enabled, a fallback pass may be attempted,
		where the `args` specified will be forwarded as leading function arguments.

		This fallback pass attempts invocation of each overload of `function`, first forwarding all of `args`,
		then truncating from the end of `args` until an invocation is successful, or until every element is exhausted.
	*/
	template <typename ArgumentContainer, typename ...Args>
	MetaAny invoke_any_overload_with_indirection_context
	(
		const MetaFunction& function, MetaAny self, const ArgumentContainer& function_arguments,
		bool handle_indirection, bool attempt_to_forward_context,
		Args&&... args
	)
	{
		using IndirectionArguments = util::small_vector<MetaAny, 8>; // MetaFunctionCall::Arguments;

		if (!function)
		{
			return {};
		}

		IndirectionArguments function_arguments_out;

		if (!function_arguments.empty())
		{
			if (handle_indirection)
			{
				function_arguments_out.reserve((function_arguments.size() + static_cast<bool>(self)));

				// NOTE: May change this back to using const references at some point. (See notes below)
				for (auto& argument : function_arguments) // const auto&
				{
					function_arguments_out.emplace_back(get_indirect_value_or_ref(argument, std::forward<Args>(args)...));
				}

				if (auto result = invoke_any_overload_with_automatic_meta_forwarding(function, self.as_ref(), function_arguments_out.data(), function_arguments_out.size()))
				{
					return result;
				}
			}
			else
			{
				// NOTE: Const-cast needed due to limitations of EnTT's API.
				if (auto result = invoke_any_overload_with_automatic_meta_forwarding(function, self.as_ref(), const_cast<MetaAny* const>(function_arguments.data()), function_arguments.size()))
				{
					return result;
				}

				// NOTE: Unlike the `handle_indirection` case, this reserve call must happen after
				// the initial invocation attempt, due to a possible (unlikely) dynamic memory allocation.
				function_arguments_out.reserve((function_arguments.size() + static_cast<bool>(self)));

				// NOTE: Non-const reference used to avoid transitive const issues with `as_ref`.
				for (auto& argument : function_arguments)
				{
					function_arguments_out.emplace_back(argument.as_ref());
				}
			}
		}

		auto execute_with_context = [&](std::size_t argument_insertion_offset=0) -> MetaAny
		{
			if constexpr (sizeof...(args) > 0)
			{
				if (attempt_to_forward_context)
				{
					//function_arguments_out.reserve(function_arguments_out.size() + sizeof...(args));

					{ std::size_t i = argument_insertion_offset; ([&] { function_arguments_out.insert(function_arguments_out.begin() + (i++), entt::forward_as_meta(args)); }(), ...); }

					for (std::size_t i = (sizeof...(args) + argument_insertion_offset); i-- > argument_insertion_offset; )
					{
						if (auto result = invoke_any_overload_with_automatic_meta_forwarding(function, self.as_ref(), function_arguments_out.data(), function_arguments_out.size())) // invoke_any_overload
						{
							return result;
						}

						function_arguments_out.erase(function_arguments_out.begin() + i);
					}
				}
			}

			return {};
		};

		if (auto result = execute_with_context())
		{
			return result;
		}

		// Try again, but with `self` handled as the first argument:
		if (self && function_has_static_overload(function))
		{
			function_arguments_out.insert(function_arguments_out.begin(), std::move(self));

			self = {};

			if (auto result = invoke_any_overload_with_automatic_meta_forwarding(function, self, function_arguments_out.data(), function_arguments_out.size()))
			{
				return result;
			}

			if (auto result = execute_with_context(1))
			{
				return result;
			}

			if (function_arguments.empty())
			{
				auto& embedded_self = function_arguments_out[0];

				if (auto result = invoke_any_overload(function, embedded_self.as_ref()))
				{
					return result;
				}
			}
		}
		else 
		{
			if (function_arguments.empty())
			{
				if (auto result = invoke_any_overload(function, self.as_ref()))
				{
					return result;
				}
			}
		}

		return {};
	}

	template <typename ...Args>
	MetaAny invoke_any_overload_with_indirection_context
	(
		const MetaFunction& function, MetaAny self,
		bool handle_indirection, bool attempt_to_forward_context,
		Args&&... args
	)
	{
		return invoke_any_overload_with_indirection_context
		(
			function,

			std::move(self),

			// Placeholder.
			// TODO: Refactor main overload to avoid temporary object.
			std::vector<MetaAny> {}, // util::small_vector<MetaAny, 1> {},

			handle_indirection,
			attempt_to_forward_context,

			std::forward<Args>(args)...
		);
	}
}