#pragma once

#include "types.hpp"
#include "indirection.hpp"

#include <util/small_vector.hpp>

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

	/*
		Attempts to invoke `function` with the `args` specified.
		
		If invocation fails, the next overload of `function` will be attempted.
		If all overloads fail, this function will return an empty `MetaAny` object.
		
		NOTE: This uses `entt::forward_as_meta` automatically when passing `args` to the active function's `invoke` method.
	*/
	template <typename InstanceType, typename ...Args>
	MetaAny invoke_any_overload(MetaFunction function, InstanceType&& instance, Args&&... args)
	{
		static_assert(std::is_same_v<std::decay_t<InstanceType>, MetaAny>, "The `instance` parameter must be a `MetaAny` object, or a reference to one.");

		while (function)
		{
			// NOTE: Function `arity` check added to avoid unnecessary object creation overhead.
			if (function.arity() == sizeof...(args)) // >=
			{
				if (auto result = function.invoke(instance, entt::forward_as_meta(args)...))
				{
					return result;
				}
			}

			function = function.next();
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

		std::size_t argument_offset = 0;

		if (function.is_static() && self)
		{
			function_arguments_out.emplace_back(std::move(self));

			argument_offset++;

			self = {};
		}

		if (!function_arguments.empty())
		{
			if (handle_indirection)
			{
				function_arguments_out.reserve(function_arguments.size());

				for (const auto& argument : function_arguments)
				{
					function_arguments_out.emplace_back(get_indirect_value_or_ref(argument, std::forward<Args>(args)...));
				}
			}
			else
			{
				if (!function.is_static())
				{
					// NOTE: Const-cast needed due to limitations of EnTT's API.
					if (auto result = invoke_any_overload_with_automatic_meta_forwarding(function, self, const_cast<MetaAny* const>(function_arguments.data()), function_arguments.size()))
					{
						return result;
					}
				}

				function_arguments_out.reserve(function_arguments.size());

				for (const auto& argument : function_arguments)
				{
					function_arguments_out.emplace_back(argument.as_ref());
				}
			}
		}

		if (function_arguments_out.size() > 0)
		{
			if (auto result = invoke_any_overload_with_automatic_meta_forwarding(function, self, function_arguments_out.data(), function_arguments_out.size()))
			{
				return result;
			}
		}
		else
		{
			if (auto result = invoke_any_overload(function, self))
			{
				return result;
			}
		}

		if constexpr (sizeof...(args) > 0)
		{
			if (attempt_to_forward_context)
			{
				{ std::size_t i = argument_offset; ([&] { function_arguments_out.insert(function_arguments_out.begin() + (i++), entt::forward_as_meta(args)); }(), ...); }

				for (std::size_t i = (sizeof...(args) + argument_offset); i-- > argument_offset; )
				{
					if (auto result = invoke_any_overload_with_automatic_meta_forwarding(function, self, function_arguments_out.data(), function_arguments_out.size())) // invoke_any_overload
					{
						return result;
					}

					function_arguments_out.erase(function_arguments_out.begin() + i);
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