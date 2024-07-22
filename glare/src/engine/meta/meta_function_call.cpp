#include "meta_function_call.hpp"

#include "meta_evaluation_context.hpp"
#include "function.hpp"
#include "indirection.hpp"
#include "container.hpp"
#include "component.hpp"
#include "traits.hpp"
#include "runtime_traits.hpp"
#include "resolve.hpp"
//#include "data_member.hpp"

#include "meta_data_member.hpp"

#include <util/algorithm.hpp>

#include <utility>

namespace engine
{
	template <typename InputContainer, typename OutputContainer, typename ...ArgumentValues>
	static void expand_arguments_to(InputContainer& input, OutputContainer& arguments_out, ArgumentValues&&... argument_values)
	{
		for (auto& argument_entry : input)
		{
			arguments_out.emplace_back(argument_entry.as_ref());
		}

		// Emplace each element of `argument_values`.
		(arguments_out.emplace_back(argument_values.as_ref()), ...); // std::move(argument_values)
	}

	template <typename OutputContainer, typename InputContainer, typename ...ArgumentValues>
	static OutputContainer expand_arguments_ex(InputContainer& input, ArgumentValues&&... argument_values)
	{
		auto arguments_out = OutputContainer {};

		expand_arguments_to(input, arguments_out, std::forward<ArgumentValues>(argument_values)...);

		return arguments_out;
	}

	template <typename InputContainer, typename ...ArgumentValues>
	static auto expand_arguments(InputContainer& input, ArgumentValues&&... argument_values)
	{
		return expand_arguments_ex<MetaFunctionCall::ForwardingArguments>(input, std::forward<ArgumentValues>(argument_values)...);
	}

	template <typename ...Args>
	MetaAny MetaFunctionCall::get_self_impl(bool resolve_as_container, bool resolve_underlying, Args&&... args) const
	{
		// NOTE: We const-cast here since `self` may need to refer to a non-const `MetaAny`.
		// (Transitive `const` forwarding from EnTT's API is not always wanted here)
		// 
		// This does not change the originating policy of `MetaFunctionCall::self`;
		// if it was `policy::cref` before, it still will be, even in a non-const calling context.
		auto& self = const_cast<MetaFunctionCall*>(this)->self;

		if (!self)
		{
			return {};
		}

		if (resolve_as_container)
		{
			if (auto as_container = try_get_container_wrapper(self)) // self_ref
			{
				return as_container;
			}
		}

		auto self_ref = self.as_ref();

		if (resolve_underlying)
		{
			if (auto self_resolved = try_get_underlying_value(self_ref, args...))
			{
				return self_resolved;
			}
		}

		return self_ref;
	}

	template <typename ...Args>
	MetaAny MetaFunctionCall::get_self(bool resolve_as_container, bool resolve_underlying, Args&&... args) const
	{
		return get_self_impl(resolve_as_container, resolve_underlying, args...);
	}

	MetaAny MetaFunctionCall::get_self(bool resolve_as_container, bool resolve_underlying) const
	{
		return get_self_impl(resolve_as_container, resolve_underlying);
	}

	template <typename ArgumentContainer, typename ...Args>
	MetaAny MetaFunctionCall::execute_impl(ArgumentContainer&& arguments, MetaAny self, bool resolve_indirection, Args&&... args) const
	{
		const bool resolve_argument_indirection = (resolve_indirection)
			? has_indirection()
			: false
		;

		auto target_fn = MetaFunction {};
		auto self_type = MetaType {};

		if (self)
		{
			if (resolve_indirection)
			{
				if (auto self_resolved = try_get_underlying_value(self, args...))
				{
					self = std::move(self_resolved);
				}
			}

			self_type = self.type();

			if (has_type())
			{
				if (self_type.id() != type_id)
				{
					return {};
				}
			}
		}
		else
		{
			self = get_self(false, resolve_indirection, args...);

			if (has_type())
			{
				self_type = get_type();
			}
			else
			{
				self_type = self.type();
			}
		}

		if (!self_type)
		{
			return {};
		}

		target_fn = get_function(self_type);

		if ((!target_fn) && (self))
		{
			if (auto as_container = try_get_container_wrapper(self))
			{
				const auto container_type = as_container.type();

				target_fn = get_function(container_type);

				if (target_fn)
				{
					self = std::move(as_container);
				}
			}
		}

		if (resolve_indirection)
		{
			if (self)
			{
				return invoke_any_overload_with_indirection_context
				(
					target_fn,
				
					(self)
						? self.as_ref() // std::move(self),
						: MetaAny {}
					,

					arguments,
					resolve_argument_indirection,
					false, // true,

					self, // self.as_ref()
					args...
				);
			}
			else
			{
				return invoke_any_overload_with_indirection_context
				(
					target_fn,
				
					std::move(self), // MetaAny {},

					arguments,
					resolve_argument_indirection,
					false, // true
					args...
				);
			}
		}
		else
		{
			return invoke_any_overload_with_indirection_context
			(
				target_fn,

				std::move(self), // self.as_ref(),

				arguments,
				resolve_argument_indirection,
				false,
				args...
			);
		}
	}

	template <typename ArgumentContainer>
	MetaAny MetaFunctionCall::execute_impl(ArgumentContainer&& arguments, Registry& registry, Entity entity, bool resolve_indirection, MetaAny self, const MetaEvaluationContext* opt_evaluation_context) const
	{
		using namespace engine::literals;

		auto target_fn = MetaFunction {};
		auto self_type = MetaType {};

		if (self)
		{
			if (resolve_indirection)
			{
				if (opt_evaluation_context)
				{
					if (auto self_resolved = try_get_underlying_value(self, registry, entity, *opt_evaluation_context))
					{
						self = std::move(self_resolved);
					}
				}
				else
				{
					if (auto self_resolved = try_get_underlying_value(self, registry, entity))
					{
						self = std::move(self_resolved);
					}
				}
			}

			self_type = self.type();
		}
		else
		{
			if (has_type())
			{
				self = get_component_ref(registry, entity, type_id);
			}

			if (self)
			{
				self_type = self.type();
			}
			else
			{
				if (opt_evaluation_context)
				{
					self = get_self(false, resolve_indirection, registry, entity, *opt_evaluation_context);
				}
				else
				{
					self = get_self(false, resolve_indirection, registry, entity);
				}

				if (has_type())
				{
					self_type = get_type();

					if ((!self) && (opt_evaluation_context))
					{
						self = opt_evaluation_context->resolve_singleton_from_type(self_type);
					}
				}
				else
				{
					self_type = self.type();
				}
			}
		}

		if (self)
		{
			if (has_type())
			{
				const auto self_type_id = self_type.id();

				if (self_type_id == "Entity"_hs) // entt::type_hash<Entity>::value()
				{
					if (auto target_entity = self.try_cast<Entity>())
					{
						if (auto as_component = get_component_ref(registry, *target_entity, type_id))
						{
							self = std::move(as_component);

							self_type = get_type();
						}
					}
				}
				else if (self_type_id != type_id)
				{
					auto type = get_type();

					if (type_has_global_namespace_flag(type))
					{
						self_type = type;
					}
					else
					{
						return {};
					}
				}
			}
		}

		if (!self_type)
		{
			return {};
		}

		target_fn = get_function(self_type);

		if ((!target_fn) && (self))
		{
			if (auto as_container = try_get_container_wrapper(self))
			{
				const auto container_type = as_container.type();

				target_fn = get_function(container_type);

				if (target_fn)
				{
					self = std::move(as_container);
				}
			}
		}

		const bool resolve_argument_indirection = (resolve_indirection)
			? has_indirection()
			: false
		;

		if (opt_evaluation_context)
		{
			return invoke_any_overload_with_indirection_context
			(
				target_fn,
				std::move(self),
				arguments,
				resolve_argument_indirection,
				true,
				registry, entity,
				*opt_evaluation_context
			);
		}
		else
		{
			return invoke_any_overload_with_indirection_context
			(
				target_fn,
				std::move(self),
				arguments,
				resolve_argument_indirection,
				true,
				registry, entity
			);
		}
	}

	template <typename ...Args>
	MetaAny MetaFunctionCall::get_fallback_impl(const MetaAny& self, bool resolve_indirection, Args&&... args) const
	{
		if (!has_type())
		{
			return {};
		}

		if (!has_function())
		{
			return {};
		}

		if (resolve_indirection)
		{
			if (auto self_resolved = try_get_underlying_value(self))
			{
				return get_fallback_impl(self_resolved, resolve_indirection, std::forward<Args>(args)...); // false
			}
		}

		const auto& member_id_from_function_id = function_id;

		auto member = MetaDataMember
		{
			type_id,
			member_id_from_function_id
		};

		if constexpr ((sizeof...(Args) > 0) && has_method_get_v<MetaDataMember, MetaAny, const MetaAny&, Args...>)
		{
			return member.get(self, std::forward<Args>(args)...);
		}
		else if constexpr (has_method_get_v<MetaDataMember, MetaAny, const MetaAny&>)
		{
			return member.get(self);
		}
		else
		{
			return {};
		}
	}

	template <typename ...Args>
	MetaAny MetaFunctionCall::get_fallback(const MetaAny& self, bool resolve_indirection, Args&&... args) const
	{
		if (!allow_fallback_to_member)
		{
			return {};
		}

		return get_fallback_impl(self, resolve_indirection, std::forward<Args>(args)...);
	}

	template <typename ...Args>
	MetaAny MetaFunctionCall::set_fallback_impl(MetaAny self, MetaAny& value, bool resolve_indirection, Args&&... args)
	{
		if (!has_type())
		{
			return {};
		}

		if (!has_function())
		{
			return {};
		}

		if (resolve_indirection)
		{
			if (auto self_resolved = try_get_underlying_value(self))
			{
				return set_fallback_impl(std::move(self_resolved), value, resolve_indirection, std::forward<Args>(args)...); // false
			}

			if (auto value_resolved = try_get_underlying_value(value))
			{
				return set_fallback_impl(self.as_ref(), value_resolved, resolve_indirection, std::forward<Args>(args)...); // false
			}
		}

		if (self)
		{
			if (auto result = set_with_fallback(value, self, resolve_indirection, args...))
			{
				return result;
			}
		}
		
		const auto& member_id_from_function_id = function_id;

		auto member = MetaDataMember
		{
			type_id,
			member_id_from_function_id
		};

		if constexpr ((sizeof...(Args) > 0) && has_method_set_v<MetaDataMember, MetaAny, MetaAny&, Args...>)
		{
			if (member.set(value, std::forward<Args>(args)...))
			{
				return entt::forward_as_meta(*this);
			}
		}
		else if constexpr (has_method_set_v<MetaDataMember, MetaAny, MetaAny&>)
		{
			if (member.set(value))
			{
				return entt::forward_as_meta(*this);
			}
		}
		
		return {};
	}

	template <typename ...Args>
	MetaAny MetaFunctionCall::set_fallback(MetaAny self, MetaAny& value, bool resolve_indirection, Args&&... args)
	{
		if (!allow_fallback_to_member)
		{
			return {};
		}

		if (this->self || this->arguments.empty())
		{
			if (auto result = set_fallback_impl(this->self.as_ref(), value, true, args...))
			{
				return result;
			}
		}

		if (this->arguments.size() == 1)
		{
			auto& first_argument = this->arguments[0];

			// TODO: Look into whether we can guarantee that a move won't occur.
			auto first_argument_out = (value_has_indirection(first_argument))
				? first_argument.as_ref()
				: MetaAny { first_argument } // Explicit copy due to potential move operation.
			;

			return set_fallback_impl(value.as_ref(), first_argument_out, true, args...);
		}

		return {};
	}

	template <typename ...Args>
	MetaAny MetaFunctionCall::set_with_fallback_impl(MetaAny& source, MetaAny& destination, bool resolve_indirection, Args&&... args)
	{
		if (!has_type())
		{
			return {};
		}

		if (!has_function())
		{
			return {};
		}

		if (resolve_indirection)
		{
			if (auto source_resolved = try_get_underlying_value(source))
			{
				return set_with_fallback_impl(source_resolved, destination, resolve_indirection, std::forward<Args>(args)...); // false
			}

			if (auto destination_resolved = try_get_underlying_value(destination))
			{
				return set_with_fallback_impl(source, destination_resolved, resolve_indirection, std::forward<Args>(args)...); // false
			}
		}

		const auto& member_id_from_function_id = function_id;

		auto member = MetaDataMember
		{
			type_id,
			member_id_from_function_id
		};

		if constexpr ((sizeof...(Args) > 0) && has_method_set_v<MetaDataMember, MetaAny, MetaAny&, MetaAny&, Args...>)
		{
			if (member.set(source, destination, std::forward<Args>(args)...))
			{
				return entt::forward_as_meta(*this);
			}
		}
		else if constexpr (has_method_set_v<MetaDataMember, MetaAny, MetaAny&, MetaAny&>)
		{
			if (member.set(source, destination))
			{
				return entt::forward_as_meta(*this);
			}
		}

		return {};
	}

	template <typename ...Args>
	MetaAny MetaFunctionCall::set_with_fallback(MetaAny& source, MetaAny& destination, bool resolve_indirection, Args&&... args)
	{
		if (!allow_fallback_to_member)
		{
			return {};
		}

		return set_with_fallback_impl(source, destination, resolve_indirection, std::forward<Args>(args)...);
	}

	MetaAny MetaFunctionCall::get(MetaAny self, bool resolve_indirection) const
	{
		if (auto result = execute_impl(arguments, ((self) ? self.as_ref() : MetaAny {}), resolve_indirection))
		{
			return result;
		}

		if (arguments.empty())
		{
			if (auto result = get_fallback(self, resolve_indirection))
			{
				return result;
			}
		}

		return {};
	}

	MetaAny MetaFunctionCall::get(MetaAny self, bool resolve_indirection, const MetaEvaluationContext& evaluation_context) const
	{
		if (auto result = execute_impl(arguments, ((self) ? self.as_ref() : MetaAny {}), resolve_indirection, evaluation_context))
		{
			return result;
		}

		if (arguments.empty())
		{
			if (auto result = get_fallback(self, resolve_indirection, evaluation_context))
			{
				return result;
			}
		}

		return {};
	}

	MetaAny MetaFunctionCall::get(Registry& registry, Entity entity, bool resolve_indirection, MetaAny self, const MetaEvaluationContext* opt_evaluation_context) const
	{
		if (auto result = execute_impl(arguments, registry, entity, resolve_indirection, self.as_ref(), opt_evaluation_context))
		{
			return result;
		}

		if (arguments.empty())
		{
			if (opt_evaluation_context)
			{
				if (auto result = get_fallback(self, resolve_indirection, registry, entity, *opt_evaluation_context))
				{
					return result;
				}
			}
			else
			{
				if (auto result = get_fallback(self, resolve_indirection, registry, entity))
				{
					return result;
				}
			}
		}

		return {};
	}

	MetaAny MetaFunctionCall::get_indirect_value() const
	{
		return get();
	}

	MetaAny MetaFunctionCall::get_indirect_value(const MetaEvaluationContext& context) const
	{
		return get({}, true, context);
	}

	MetaAny MetaFunctionCall::get_indirect_value(const MetaAny& self) const
	{
		// NOTE: Const-cast needed here to allow for non-const member-function calls.
		// Usage of `as_ref` allows for reference-policy forwarding without
		// imposing transitive const from reference-type of `self`.
		return get(const_cast<MetaAny&>(self).as_ref(), true);
	}

	MetaAny MetaFunctionCall::get_indirect_value(const MetaAny& self, Registry& registry, Entity entity) const
	{
		// NOTE: See basic indirect chain-getter implementation for details on const-cast usage.
		return get(registry, entity, true, const_cast<MetaAny&>(self).as_ref());
	}

	MetaAny MetaFunctionCall::get_indirect_value(const MetaAny& self, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		// NOTE: See basic indirect chain-getter implementation for details on const-cast usage.
		return get(registry, entity, true, const_cast<MetaAny&>(self).as_ref(), &context);
	}

	MetaAny MetaFunctionCall::get_indirect_value(Registry& registry, Entity entity) const
	{
		return get(registry, entity, true);
	}

	MetaAny MetaFunctionCall::get_indirect_value(Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return get(registry, entity, true, {}, &context);
	}


	MetaAny MetaFunctionCall::set_indirect_value(MetaAny& value)
	{
		auto arguments_out = expand_arguments(this->arguments, value);

		if (auto result = execute_impl(arguments_out))
		{
			return result;
		}

		return set_fallback(this->self.as_ref(), value, true);
	}

	MetaAny MetaFunctionCall::set_indirect_value(MetaAny& value, const MetaEvaluationContext& context)
	{
		// TODO: Implement `context`-only overload for `execute_impl`.
		// 
		// Defaulting to contextless version for now.
		return set_indirect_value(value);
	}

	MetaAny MetaFunctionCall::set_indirect_value(MetaAny& value, Registry& registry, Entity entity)
	{
		auto arguments_out = expand_arguments(this->arguments, value);

		if (auto result = execute_impl(arguments_out, registry, entity))
		{
			return result;
		}

		return set_fallback(this->self.as_ref(), value, true, registry, entity);
	}

	MetaAny MetaFunctionCall::set_indirect_value(MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		auto arguments_out = expand_arguments(this->arguments, value);

		if (auto result = execute_impl(arguments_out, registry, entity, true, {}, &context))
		{
			return result;
		}

		return set_fallback(this->self.as_ref(), value, true, registry, entity, context);
	}

	MetaAny MetaFunctionCall::set_indirect_value(MetaAny& source, MetaAny& destination)
	{
		auto arguments_out = expand_arguments(this->arguments, source, destination);

		if (auto result = execute_impl(arguments_out))
		{
			return result;
		}

		return set_with_fallback(source, destination, true);
	}

	MetaAny MetaFunctionCall::set_indirect_value(MetaAny& source, MetaAny& destination, const MetaEvaluationContext& context)
	{
		// TODO: Implement `context`-only overload for `execute_impl`.
		// 
		// Defaulting to contextless version for now.
		return set_indirect_value(source, destination);
	}

	MetaAny MetaFunctionCall::set_indirect_value(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity)
	{
		auto arguments_out = expand_arguments(this->arguments, source, destination);

		if (auto result = execute_impl(arguments_out, registry, entity))
		{
			return result;
		}

		return set_with_fallback(source, destination, true, registry, entity);
	}

	MetaAny MetaFunctionCall::set_indirect_value(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		auto arguments_out = expand_arguments(this->arguments, source, destination);

		if (auto result = execute_impl(arguments_out, registry, entity, true, {}, &context))
		{
			return result;
		}

		return set_with_fallback(source, destination, true, registry, entity, context);
	}

	bool MetaFunctionCall::has_indirection() const
	{
		for (const auto& value : arguments)
		{
			if (value_has_indirection(value))
			{
				return true;
			}
		}

		return false;
	}

	MetaFunction MetaFunctionCall::get_function() const
	{
		return get_function(get_type());
	}

	MetaFunction MetaFunctionCall::get_function(const MetaAny& self) const
	{
		if (self)
		{
			return get_function(self.type());
		}

		return get_function();
	}

	MetaFunction MetaFunctionCall::get_function(const MetaType& type) const
	{
		if (!type)
		{
			return {};
		}

		return type.func(function_id);
	}

	MetaType MetaFunctionCall::get_type() const
	{
		if (has_type())
		{
			return resolve(type_id);
		}

		return {};
	}

	bool MetaFunctionCall::has_type() const
	{
		return static_cast<bool>(type_id);
	}

	bool MetaFunctionCall::has_function() const
	{
		return static_cast<bool>(function_id);
	}
}