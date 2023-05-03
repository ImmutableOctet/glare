#include "meta_property.hpp"

#include "indirection.hpp"
#include "function.hpp"
#include "hash.hpp"
#include "traits.hpp"

#include "meta_data_member.hpp"
#include "meta_evaluation_context.hpp"

#include <util/small_vector.hpp>

namespace engine
{
	template <typename ...Args>
	MetaAny MetaProperty::get_fallback_impl(Args&&... args) const
	{
		if constexpr (has_method_get_v<MetaDataMember, MetaAny, Args...>)
		{
			if (has_data_member()) // && has_type()
			{
				auto member = MetaDataMember { type_id, data_member_id };

				return member.get(std::forward<Args>(args)...);
			}
		}

		return {};
	}

	template <typename ...Args>
	MetaAny MetaProperty::set_fallback_impl(Args&&... args)
	{
		if constexpr (has_method_set_v<MetaDataMember, MetaAny, Args...>)
		{
			if (has_data_member()) // && has_type()
			{
				auto member = MetaDataMember { type_id, data_member_id };

				if (member.set(std::forward<Args>(args)...))
				{
					return entt::forward_as_meta(*this);
				}
			}
		}

		return {};
	}

	template <bool check_type, typename ...Args>
	MetaAny MetaProperty::get_from_impl(const MetaAny& instance, Args&&... args) const
	{
		if (!has_getter())
		{
			return {};
		}

		auto instance_type = MetaType {};

		if constexpr (check_type)
		{
			instance_type = instance.type();

			if (has_type() && (instance_type.id() != type_id))
			{
				return {};
			}
		}
		else
		{
			instance_type = get_type();
		}

		return get_from_impl_ex(instance, instance_type, std::forward<Args>(args)...);
	}

	template <typename ...Args>
	MetaAny MetaProperty::get_from_impl_ex(const MetaAny& instance, const MetaType& instance_type, Args&&... args) const
	{
		return invoke_any_overload_with_indirection_context
		(
			getter(instance_type),
			
			// NOTE: Const-cast used to avoid unwanted transitive
			// const applied to temporary reference object.
			const_cast<MetaAny&>(instance).as_ref(),

			true, (sizeof...(args) > 0),
			
			std::forward<Args>(args)...
		);
	}

	template <bool fallback_to_entity_type, typename ...Args>
	MetaAny MetaProperty::get_from_entity_impl(Entity target, Registry& registry, Args&&... args) const
	{
		using namespace engine::literals;

		if (!has_getter())
		{
			return {};
		}

		auto type = get_type();

		if (!type)
		{
			if constexpr (fallback_to_entity_type)
			{
				type = resolve<Entity>();
			}
			else
			{
				return {};
			}
		}

		if (type.id() == "Entity"_hs) // (type == resolve<Entity>())
		{
			return get_from_impl_ex(entt::forward_as_meta(target), type, registry, std::forward<Args>(args)...);
		}
		else if (auto instance = MetaDataMember::get_instance(type, registry, target))
		{
			return get_from_impl_ex(instance, type, registry, std::forward<Args>(args)...);
		}

		return {};
	}

	template <bool check_value_type, typename ...Args>
	MetaAny MetaProperty::static_set_impl(MetaAny& value, Args&&... args)
	{
		if (!has_type())
		{
			return {};
		}

		return static_set_impl_ex<check_value_type>(get_type(), value, std::forward<Args>(args)...);
	}

	template <bool check_value_type, typename ...Args>
	MetaAny MetaProperty::static_set_impl_ex(const MetaType& type, MetaAny& value, Args&&... args)
	{
		if (!has_setter())
		{
			return {};
		}

		if (!type)
		{
			return {};
		}

		if (!value)
		{
			return {};
		}

		if constexpr (check_value_type)
		{
			if (const auto member_type = get_member_type())
			{
				if (!value_has_indirection(value))
				{
					if (value.type() != member_type)
					{
						return {};
					}
				}
			}
		}

		auto setter_arguments = util::small_vector<MetaAny, 1>
		{
			value.as_ref() // std::move(value)
		};

		auto result = invoke_any_overload_with_indirection_context
		(
			setter(type),
			MetaAny {},
			setter_arguments,
			true, (sizeof...(args) > 0),
			std::forward<Args>(args)...
		);

		if (!result)
		{
			return {};
		}

		return entt::forward_as_meta(*this); // result;
	}

	template <bool fallback_to_entity_type, bool check_value_type, typename ...Args>
	MetaAny MetaProperty::set_impl(MetaAny& value, Registry& registry, Entity entity, Entity context_entity, Args&&... args)
	{
		using namespace engine::literals;

		if (!has_setter())
		{
			return {};
		}

		if constexpr (check_value_type)
		{
			if (const auto member_type = get_member_type())
			{
				if (!value_has_indirection(value))
				{
					if (value.type() != member_type)
					{
						return {};
					}
				}
			}
		}

		auto type = get_type();

		if (!type)
		{
			if constexpr (fallback_to_entity_type)
			{
				type = resolve<Entity>();
			}
			else
			{
				return {};
			}
		}

		if (type.id() == "Entity"_hs) // (type == resolve<Entity>())
		{
			auto entity_as_destination = MetaAny { entity }; // entt::forward_as_meta(entity);

			return set_with_impl_ex<false, false>(value, entity_as_destination, type, registry, context_entity, std::forward<Args>(args)...);
		}
		else if (auto instance = MetaDataMember::get_instance(type, registry, entity))
		{
			return set_with_impl_ex<false, false>(value, instance, type, registry, context_entity, std::forward<Args>(args)...);
		}

		return static_set_impl_ex<false>(type, value, registry, context_entity, std::forward<Args>(args)...);
	}

	template <bool check_source_type, bool check_destination_type, typename ...Args>
	MetaAny MetaProperty::set_with_impl(MetaAny& source, MetaAny& destination, Args&&... args)
	{
		if (!destination)
		{
			return {};
		}

		const auto destination_type = destination.type();

		return set_with_impl_ex<check_source_type, check_destination_type>(source, destination, destination_type, std::forward<Args>(args)...);
	}

	template <bool check_source_type, bool check_destination_type, typename ...Args>
	MetaAny MetaProperty::set_with_impl_ex(MetaAny& source, MetaAny& destination, const MetaType& destination_type, Args&&... args)
	{
		if (!has_setter())
		{
			return {};
		}

		if (!destination)
		{
			return {};
		}

		if (!source)
		{
			return {};
		}

		if (!destination_type)
		{
			return {};
		}

		if constexpr (check_destination_type)
		{
			if (has_type())
			{
				if (destination_type.id() != type_id)
				{
					return {};
				}
			}
		}

		if constexpr (check_source_type)
		{
			if (const auto member_type = get_member_type())
			{
				if (!value_has_indirection(source))
				{
					if (source.type() != member_type)
					{
						return {};
					}
				}
			}
		}

		auto setter_arguments = util::small_vector<MetaAny, 1>
		{
			source.as_ref() // std::move(source)
		};

		auto result = invoke_any_overload_with_indirection_context
		(
			setter(destination_type),
			destination.as_ref(),
			setter_arguments,
			true, (sizeof...(args) > 0),
			std::forward<Args>(args)...
		);

		if (!result)
		{
			return {};
		}

		return entt::forward_as_meta(*this); // result;
	}

	MetaAny MetaProperty::get() const
	{
		if (has_getter())
		{
			if (auto result = invoke_any_overload_with_indirection_context(getter(), MetaAny{}, false, false))
			{
				return result;
			}
		}

		return get_fallback_impl();
	}

	MetaAny MetaProperty::get(const MetaEvaluationContext& context) const
	{
		if (has_getter())
		{
			if (auto result = invoke_any_overload_with_indirection_context(getter(), MetaAny{}, false, true, context))
			{
				return result;
			}
		}

		return get_fallback_impl(context);
	}

	MetaAny MetaProperty::get(const MetaAny& instance) const
	{
		if (auto result = get_from_impl<true>(instance))
		{
			return result;
		}

		return get_fallback_impl(instance);
	}

	MetaAny MetaProperty::get(const MetaAny& instance, const MetaEvaluationContext& context) const
	{
		if (auto result = get_from_impl<true>(instance, context))
		{
			return result;
		}

		return get_fallback_impl(instance, context);
	}

	MetaAny MetaProperty::get(Registry& registry, Entity entity) const
	{
		if (auto entity_or_component_result = get(entity, registry, entity))
		{
			return entity_or_component_result;
		}

		// Attempt to execute `getter` again as a static member-function.
		if (auto result = invoke_any_overload_with_indirection_context(getter(), MetaAny{}, true, true, registry, entity))
		{
			return result;
		}

		return get_fallback_impl(registry, entity);
	}

	MetaAny MetaProperty::get(const MetaAny& instance, Registry& registry, Entity context_entity) const
	{
		if (auto result = get_from_impl<true>(instance, registry, context_entity))
		{
			return result;
		}

		return get_fallback_impl(instance, registry, context_entity);
	}

	MetaAny MetaProperty::get(const MetaAny& instance, Registry& registry, Entity context_entity, const MetaEvaluationContext& context) const
	{
		if (auto result = get_from_impl<true>(instance, registry, context_entity, context))
		{
			return result;
		}

		return get_fallback_impl(instance, registry, context_entity, context);
	}

	MetaAny MetaProperty::get(Entity target, Registry& registry, Entity context_entity) const
	{
		if (auto result = get_from_entity_impl<true>(target, registry, context_entity))
		{
			return result;
		}

		return get_fallback_impl(target, registry, context_entity);
	}

	MetaAny MetaProperty::get(Entity target, Registry& registry, Entity context_entity, const MetaEvaluationContext& context) const
	{
		if (auto result = get_from_entity_impl<true>(target, registry, context_entity, context))
		{
			return result;
		}

		return get_fallback_impl(target, registry, context_entity, context);
	}

	MetaAny MetaProperty::set(MetaAny& value)
	{
		if (auto result = static_set_impl<true>(value))
		{
			return result;
		}

		return set_fallback_impl(value);
	}

	MetaAny MetaProperty::set(MetaAny& value, const MetaEvaluationContext& context)
	{
		if (auto result = static_set_impl<true>(value, context))
		{
			return result;
		}

		return set_fallback_impl(value, context);
	}

	MetaAny MetaProperty::set(MetaAny& source, MetaAny& destination)
	{
		if (auto result = set_with_impl<true, true>(source, destination))
		{
			return result;
		}

		return set_fallback_impl(source, destination);
	}

	MetaAny MetaProperty::set(MetaAny& source, MetaAny& destination, const MetaEvaluationContext& context)
	{
		if (auto result = set_with_impl<true, true>(source, destination, context))
		{
			return result;
		}

		return set_fallback_impl(source, destination, context);
	}

	MetaAny MetaProperty::set(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity)
	{
		if (auto result = set_with_impl<true, true>(source, destination, registry, entity))
		{
			return result;
		}

		return set_fallback_impl(source, destination, registry, entity);
	}

	MetaAny MetaProperty::set(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		if (auto result = set_with_impl<true, true>(source, destination, registry, entity, context))
		{
			return result;
		}

		return set_fallback_impl(source, destination, registry, entity, context);
	}

	MetaAny MetaProperty::set(MetaAny& value, Registry& registry, Entity entity)
	{
		if (auto result = set_impl<true, true>(value, registry, entity, entity))
		{
			return result;
		}

		return set_fallback_impl(value, registry, entity);
	}

	MetaAny MetaProperty::set(MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		if (auto result = set_impl<true, true>(value, registry, entity, entity, context))
		{
			return result;
		}

		return set_fallback_impl(value, registry, entity, context);
	}

	bool MetaProperty::has_type() const
	{
		return static_cast<bool>(type_id);
	}

	MetaType MetaProperty::get_type() const
	{
		if (!type_id)
		{
			return {};
		}

		return resolve(type_id);
	}

	bool MetaProperty::has_member_type() const
	{
		return (has_type() && (has_getter() || has_data_member()));
	}

	bool MetaProperty::has_data_member() const
	{
		return static_cast<bool>(data_member_id);
	}

	MetaType MetaProperty::get_member_type() const
	{
		if (auto type = get_type())
		{
			if (auto getter_fn = getter(type))
			{
				return getter_fn.ret();
			}

			if (auto member = data_member(type))
			{
				return member.type();
			}
		}

		return {};
	}

	bool MetaProperty::has_member_access() const
	{
		return (has_getter() || has_setter());
	}

	bool MetaProperty::has_getter() const
	{
		return static_cast<bool>(getter_id);
	}

	bool MetaProperty::has_setter() const
	{
		return static_cast<bool>(setter_id);
	}

	MetaFunction MetaProperty::getter(const MetaType& type) const
	{
		if (!type)
		{
			return {};
		}

		if (!has_getter())
		{
			return {};
		}

		return type.func(getter_id);
	}

	MetaFunction MetaProperty::getter() const
	{
		if (!has_getter())
		{
			return {};
		}

		return getter(get_type());
	}

	MetaFunction MetaProperty::setter(const MetaType& type) const
	{
		if (!type)
		{
			return {};
		}

		if (!has_setter())
		{
			return {};
		}

		return type.func(setter_id);
	}

	MetaFunction MetaProperty::setter() const
	{
		if (!has_setter())
		{
			return {};
		}

		return setter(get_type());
	}

	entt::meta_data MetaProperty::data_member(const MetaType& type) const
	{
		if (type && has_data_member())
		{
			return type.data(data_member_id);
		}

		return {};
	}

	entt::meta_data MetaProperty::data_member() const
	{
		if (!has_type() || !has_data_member())
		{
			return {};
		}

		return data_member(get_type());
	}
}