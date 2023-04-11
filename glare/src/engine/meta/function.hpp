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
	MetaAny invoke_any_overload(MetaFunction function, InstanceType&& instance, MetaAny* const args, std::size_t arg_count)
	{
		static_assert(std::is_same_v<std::decay_t<InstanceType>, MetaAny>, "The `instance` parameter must be a `MetaAny` object, or a reference to one.");

		while (function)
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

			function = function.next();
		}

		return {};
	}

    // Thia function attempts to invoke an overload of `function` that takes `arg_count` from `args`.
    // If this fails, a second invocation pass will be attempted where any overload's parameters
    // of type `MetaAny` is handled via an `entt::forward_as_meta` passthrough.
    template <typename InstanceType>
	static MetaAny invoke_any_overload_with_automatic_meta_forwarding(MetaFunction function, InstanceType&& instance, MetaAny* const args, std::size_t arg_count)
	{
		static_assert(std::is_same_v<std::decay_t<InstanceType>, MetaAny>, "The `instance` parameter must be a `MetaAny` object, or a reference to one.");

		if (auto result = invoke_any_overload(function, instance, args, arg_count))
		{
			return result;
		}

		using TemporaryArgumentStore = util::small_vector<MetaAny, 8>;

		TemporaryArgumentStore forwarding_store;

		while (function)
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

			function = function.next();
		}

		return {};
	}
}