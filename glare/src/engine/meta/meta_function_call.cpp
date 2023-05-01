#include "meta_function_call.hpp"

#include "meta_evaluation_context.hpp"
#include "function.hpp"
#include "indirection.hpp"
#include "container.hpp"
#include "component.hpp"
//#include "data_member.hpp"

#include <util/algorithm.hpp>

#include <utility>

namespace engine
{
	template <typename ...Args>
	MetaAny MetaFunctionCall::get_self_impl(bool resolve_as_container, bool resolve_underlying, Args&&... args) const
	{
		auto& self = const_cast<MetaFunctionCall*>(this)->self;

		if (!self)
		{
			return {};
		}

		// NOTE: We const-cast here since `self` may need to refer to a non-const `MetaAny`.
		// (Transitive `const` forwarding from EnTT's API is not always wanted here)
		// 
		// This does not change the originating policy of `MetaFunctionCall::self`;
		// if it was `policy::cref` before, it still will be, even in a non-const calling context.

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
			if (auto self_resolved = try_get_underlying_value(self_ref, std::forward<Args>(args)...))
			{
				return self_resolved;
			}
		}

		return self_ref;
	}

	template <typename ...Args>
	MetaAny MetaFunctionCall::get_self(bool resolve_as_container, bool resolve_underlying, Args&&... args) const
	{
		return get_self_impl(resolve_as_container, resolve_underlying, std::forward<Args>(args)...);
	}

	MetaAny MetaFunctionCall::get_self(bool resolve_as_container, bool resolve_underlying) const
	{
		return get_self_impl(resolve_as_container, resolve_underlying);
	}

	MetaAny MetaFunctionCall::get(MetaAny self, bool resolve_indirection) const
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
				if (auto self_resolved = try_get_underlying_value(self))
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
			self = get_self(false, resolve_indirection);

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

					self // self.as_ref()
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
					false // true
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
				false
			);
		}
	}

	MetaAny MetaFunctionCall::get(Registry& registry, Entity entity, bool resolve_indirection, MetaAny self, const MetaEvaluationContext* opt_evaluation_context) const
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
						self = get_component_ref(registry, *target_entity, type_id);

						self_type = get_type();
					}
				}
				else if (self_type_id != type_id)
				{
					return {};
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
}