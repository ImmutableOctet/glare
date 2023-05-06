#include "meta_type_reference.hpp"

#include "hash.hpp"
#include "apply_operation.hpp"
#include "indirection.hpp"

#include "meta_evaluation_context.hpp"

#include <engine/system_manager_interface.hpp>

//#include <entt/meta/meta.hpp>

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
		if (!instance)
		{
			return {};
		}

		const auto component_type = instance.type();

		return set_instance(component_type, std::move(instance), registry, entity);
	}

	MetaAny MetaTypeReference::set_instance(const MetaType& component_type, MetaAny&& instance, Registry& registry, Entity entity)
	{
		using namespace engine::literals;

		if (!instance)
		{
			return {};
		}

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

	/*
	// Disabled for now; may revisit again later.
	MetaAny MetaTypeReference::get() const
	{
		if (auto type = get_type())
		{
			return type.construct();
		}

		return {};
	}
	*/

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
		return get_from_entity_impl(entity, registry);
	}

	MetaAny MetaTypeReference::get(Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		if (auto result = get(registry, entity))
		{
			return result;
		}

		return get(context);
	}

	MetaAny MetaTypeReference::get(Entity target, Registry& registry, Entity context_entity) const
	{
		return get_from_entity_impl(target, registry);
	}

	MetaAny MetaTypeReference::get(Entity target, Registry& registry, Entity context_entity, const MetaEvaluationContext& context) const
	{
		return get_from_entity_impl(target, registry);
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
		using namespace engine::literals;

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

		if (instance_type_id == "Entity"_hs) // entt::type_hash<Entity>::value(): // resolve<Entity>().id()
		{
			if (const auto target_entity = instance.try_cast<Entity>())
			{
				// NOTE: Direct usage of `get` avoided here to prevent possibility of infinite recursion.
				// 
				// Explanation: There aren't enough overload-permutations for `get` directly taking an `Entity`,
				// so ADL may fallback to an implicit conversion to `MetaAny`, which would recurse to this exact spot.
				if (auto result = self.get_from_entity_impl(*target_entity, args...))
				{
					return result;
				}
			}
		}

		if (instance_type_id == self.type_id)
		{
			return instance.as_ref(); // self.get(instance);
		}

		if (auto resolved = try_get_underlying_value(instance, args...))
		{
			// NOTE: Recursion.
			return get_from_impl(self, resolved, args...);
		}

		return {};
	}

	template <typename ...Args>
	MetaAny MetaTypeReference::get_from_entity_impl(Entity entity, Registry& registry, Args&&... args) const
	{
		return get_instance(this->type_id, registry, entity);
	}

	MetaAny MetaTypeReference::get(const MetaEvaluationContext& context) const
	{
		if (context.system_manager)
		{
			const auto type = get_type();

			if (type_is_system(type))
			{
				if (auto system_handle = context.system_manager->get_system_handle(type.id()))
				{
					return *system_handle.ptr;
				}
			}
		}

		//return get();
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
		if (!instance)
		{
			return {};
		}

		if (validate_assignment_type)
		{
			if (has_type())
			{
				const auto instance_type = instance.type();

				if (instance_type.id() != this->type_id)
				{
					return {};
				}
			}
		}

		// Alternative implementation:
		//auto existing = get_instance(this->type_id, registry, entity);
		//return set(instance, existing);

		const auto type = get_type();

		if (auto result = set_instance(((type) ? type : instance.type()), instance.as_ref(), registry, entity)) // std::move(instance)
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
		using namespace engine::literals;

		const auto source_type = source.type();

		if (source_type.id() == "Entity"_hs) // entt::type_hash<Entity>::value()
		{
			if (auto source_entity = source.try_cast<Entity>())
			{
				auto copy_of_source_component = MetaAny { get_instance(this->type_id, registry, *source_entity) };

				return set(copy_of_source_component, destination, registry, entity);
			}
		}

		const auto destination_type = destination.type();

		if (destination_type.id() == "Entity"_hs) // entt::type_hash<Entity>::value()
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

	bool MetaTypeReference::is_system_type() const
	{
		if (auto type = get_type())
		{
			return type_is_system(type);
		}

		return false;
	}
}