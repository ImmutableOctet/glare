#pragma once

#include "types.hpp"

#include <util/small_vector.hpp>

#include <type_traits>

namespace engine
{
	bool function_overload_has_meta_any_argument(const MetaFunction& function);
	bool function_has_meta_any_argument(const MetaFunction& function, bool check_overloads=true);

	// Attempts to invoke `function` with the `args` specified.
    // 
    // If invocation fails, the next overload of `function` will be attempted.
    // If all overloads fail, this function will return an empty `MetaAny` object.
    // 
    // NOTE: This function encapsulates `args` via `entt::forward_as_meta` automatically.
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

    // Thia function attempts to invoke an overload of `function` that takes `arg_count` from `args`.
    // If this fails, a second invocation pass will be attempted where any overload's parameters
    // of type `MetaAny` is handled via an `entt::forward_as_meta` passthrough.
    template <typename InstanceType>
	static MetaAny invoke_any_overload_with_automatic_meta_forwarding(MetaFunction function_overloads, InstanceType&& instance, MetaAny* const args, std::size_t arg_count)
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
}