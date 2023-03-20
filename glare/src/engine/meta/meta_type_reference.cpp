#include "meta_type_reference.hpp"

#include "meta.hpp"
#include "apply_operation.hpp"

#include <utility>

namespace engine
{
	MetaAny MetaTypeReference::get_instance(MetaTypeID type_id, Registry& registry, Entity entity)
	{
		using namespace engine::literals;

		auto type = resolve(type_id);

		if (!type)
		{
			return {};
		}

		auto get_fn = type.func("get_component"_hs);

		if (!get_fn)
		{
			return {};
		}

		auto instance_ptr = get_fn.invoke
		(
			{},
			entt::forward_as_meta(registry),
			entt::forward_as_meta(entity)
		);

		if (!instance_ptr)
		{
			return {};
		}

		const auto instance = *instance_ptr;

		if (!instance)
		{
			return {};
		}

		return instance;
	}

	MetaAny MetaTypeReference::set_instance(MetaAny&& instance, Registry& registry, Entity entity)
	{
		using namespace engine::literals;

		if (!instance)
		{
			return {};
		}

		const auto component_type = instance.type();

		const auto replace_fn = component_type.func("emplace_meta_component"_hs);

		if (!replace_fn)
		{
			return {};
		}

		return replace_fn.invoke
		(
			{},

			entt::forward_as_meta(registry),
			entt::forward_as_meta(entity),
			entt::forward_as_meta(std::move(instance))
		);
	}

	MetaAny MetaTypeReference::get(const MetaAny& instance) const
	{
		if (instance)
		{
			if (const auto instance_type = instance.type())
			{
				if (instance_type.id() == type_id)
				{
					return instance.as_ref();
				}
			}
		}

		return {};
	}

	MetaAny MetaTypeReference::get(Registry& registry, Entity entity) const
	{
		return get_instance(this->type_id, registry, entity);
	}

	MetaAny MetaTypeReference::get(Entity target, Registry& registry, Entity context_entity) const
	{
		return get(registry, target);
	}

	MetaAny MetaTypeReference::get(const MetaAny& instance, Registry& registry, Entity context_entity) const
	{
		return get_from_impl(*this, instance, registry, context_entity);
	}

	MetaAny MetaTypeReference::get(const MetaAny& instance, Registry& registry, Entity context_entity, const MetaEvaluationContext& context) const
	{
		return get_from_impl(*this, instance, registry, context_entity, context);
	}

	template <typename SelfType, typename InstanceType, typename ...Args>
	MetaAny MetaTypeReference::get_from_impl(SelfType&& self, InstanceType&& instance, Args&&... args)
	{
		if (!instance)
		{
			return {}; // self.get(registry, entity);
		}

		const auto instance_type = instance.type();

		if (!instance_type)
		{
			return {};
		}

		const auto instance_type_id = instance_type.id();

		switch (instance_type_id)
		{
			case entt::type_hash<Entity>::value(): // resolve<Entity>().id():
				if (auto target_entity = instance.try_cast<Entity>())
				{
					return self.get(*target_entity, args...);
				}

				break;
			default:
				if (instance_type_id == self.type_id)
				{
					return instance.as_ref(); // self.get(instance);
				}
				
				if (auto resolved = try_get_underlying_value(instance, args...))
				{
					// NOTE: Recursion.
					return get_from_impl(self, resolved, args...);
				}

				break;
		}

		return {};
	}

	MetaAny MetaTypeReference::set(MetaAny& source, MetaAny& destination)
	{
		if (!source || !destination)
		{
			return {};
		}

		auto destination_type = destination.type();

		if ((!destination_type) || (destination_type.id() != this->type_id))
		{
			return {};
		}

		if (auto result = apply_operation(destination, source, MetaValueOperator::Assign))
		{
			return entt::forward_as_meta(*this); // result;
		}

		return {};
	}

	MetaAny MetaTypeReference::set(MetaAny& instance, Registry& registry, Entity entity)
	{
		const auto instance_type = instance.type();
		const auto instance_type_id = instance_type.id();

		if (instance_type_id != this->type_id)
		{
			return {};
		}

		// Alternative implementation:
		//auto existing = get_instance(this->type_id, registry, entity);
		//return set(instance, existing);

		if (auto result = set_instance(std::move(instance), registry, entity))
		{
			return entt::forward_as_meta(*this); // result;
		}

		return {};
	}

	MetaAny MetaTypeReference::set(MetaAny& instance, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		return set(instance, registry, entity);
	}

	MetaAny MetaTypeReference::set(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity)
	{
		const auto source_type = source.type();

		if (source_type.id() == entt::type_hash<Entity>::value())
		{
			if (auto source_entity = source.try_cast<Entity>())
			{
				auto copy_of_source_component = MetaAny { get_instance(this->type_id, registry, *source_entity) };

				return set(copy_of_source_component, destination, registry, entity);
			}
		}

		const auto destination_type = destination.type();

		if (destination_type.id() == entt::type_hash<Entity>::value())
		{
			if (auto destination_entity = destination.try_cast<Entity>())
			{
				return set(source, registry, *destination_entity);
			}
		}
		else
		{
			return set(source, destination);
		}

		return {};
	}

	MetaAny MetaTypeReference::set(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		return set(source, destination, registry, entity);
	}

	bool MetaTypeReference::has_type() const
	{
		return static_cast<bool>(type_id);
	}

	MetaType MetaTypeReference::get_type() const
	{
		if (has_type())
		{
			return resolve(type_id);
		}

		return {};
	}
}