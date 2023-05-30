#include "component.hpp"

#include "hash.hpp"
//#include "function.hpp"

#include "meta_type_descriptor.hpp"
#include "meta_evaluation_context.hpp"

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	MetaAny get_component_ref(Registry& registry, Entity entity, const MetaType& component_type)
	{
		using namespace engine::literals;

		if (!component_type)
		{
			return {};
		}

		if (entity == null)
		{
			return {};
		}

		auto get_component_fn = component_type.func("get_component"_hs);

		while (get_component_fn)
		{
			auto component_ptr = get_component_fn.invoke // invoke_any_overload
			(
				{},
				entt::forward_as_meta(registry),
				entt::forward_as_meta(entity)
			);

			if (component_ptr)
			{
				return *component_ptr;
			}

			get_component_fn = get_component_fn.next();
		}

		return {};
	}

	MetaAny get_component_ref(Registry& registry, Entity entity, MetaTypeID component_type_id)
	{
		return get_component_ref(registry, entity, resolve(component_type_id));
	}

	MetaAny get_or_emplace_component(Registry& registry, Entity entity, const MetaType& component_type)
	{
		using namespace engine::literals;

		/*
		// Alternative implementation:
		if (auto existing_component = get_component_ref(registry, entity, component_type))
		{
			return existing_component;
		}

		return emplace_default_component(registry, entity, component_type);
		*/

		if (!component_type)
		{
			return {};
		}

		if (entity == null)
		{
			return {};
		}

		auto emplace_component_fn = component_type.func("get_or_default_construct_component"_hs);

		while (emplace_component_fn)
		{
			auto component_ptr = emplace_component_fn.invoke // invoke_any_overload
			(
				{},
				entt::forward_as_meta(registry),
				entt::forward_as_meta(entity)
			);

			if (component_ptr)
			{
				return *component_ptr;
			}

			emplace_component_fn = emplace_component_fn.next();
		}

		return {};
	}

	MetaAny get_or_emplace_component(Registry& registry, Entity entity, MetaTypeID component_type_id)
	{
		return get_or_emplace_component(registry, entity, resolve(component_type_id));
	}

	// Attempts to emplace a default-constructed instance of `component_type` to `entity`.
	MetaAny emplace_default_component(Registry& registry, Entity entity, const MetaType& component_type)
	{
		using namespace engine::literals;

		if (!component_type)
		{
			return {};
		}

		if (entity == null)
		{
			return {};
		}

		auto default_component_fn = component_type.func("emplace_default_component"_hs);

		while (default_component_fn)
		{
			auto component_ptr = default_component_fn.invoke // invoke_any_overload
			(
				{},
				entt::forward_as_meta(registry),
				entt::forward_as_meta(entity)
			);

			if (component_ptr)
			{
				return *component_ptr;
			}

			default_component_fn = default_component_fn.next();
		}

		return {};
	}

	MetaAny emplace_default_component(Registry& registry, Entity entity, MetaTypeID component_type_id)
	{
		return emplace_default_component(registry, entity, resolve(component_type_id));
	}

	MetaAny emplace_component
	(
		Registry& registry, Entity entity,
		const MetaTypeDescriptor& component,
		const MetaEvaluationContext* opt_evaluation_context
	)
	{
		using namespace engine::literals;

		auto instance = MetaAny {};

		if (opt_evaluation_context)
		{
			instance = component.instance(true, registry, entity, *opt_evaluation_context);
		}
		else
		{
			instance = component.instance(true, registry, entity);
		}

		//assert(instance);

		auto result = MetaAny {};

		if (instance)
		{
			const auto type = component.get_type();

			if (auto emplace_fn = type.func("emplace_meta_component"_hs))
			{
				result = emplace_fn.invoke
				(
					{},

					entt::forward_as_meta(registry),
					entt::forward_as_meta(entity),
					entt::forward_as_meta(std::move(instance)) // entt::forward_as_meta(instance)
				);
			}

			if (!result)
			{
				print_warn("Failed to attach component: #{} to Entity #{}", component.get_type_id(), entity);
			}
		}
		else
		{
			print_warn("Failed to instantiate component: #{} for Entity #{}", component.get_type_id(), entity);
		}

		return result;
	}

	bool update_component_fields
	(
		Registry& registry, Entity entity,
		const MetaTypeDescriptor& component,
		
		bool value_assignment, bool direct_modify,
		const MetaEvaluationContext* opt_evaluation_context
	)
	{
		using namespace engine::literals;

		auto type = component.get_type();

		if (value_assignment && (!direct_modify))
		{
			auto patch_fn = type.func("indirect_patch_meta_component"_hs);

			if (!patch_fn)
			{
				print_warn("Unable to resolve patch function for: #{}", type.id());

				return false;
			}

			if (component.size() > 0)
			{
				auto result = patch_fn.invoke
				(
					{},
					
					entt::forward_as_meta(registry),
					entt::forward_as_meta(entity),
					entt::forward_as_meta(component),
					entt::forward_as_meta(component.size()),
					entt::forward_as_meta(0),
					entt::forward_as_meta(Entity(null)),
					entt::forward_as_meta(opt_evaluation_context),
					entt::forward_as_meta(MetaAny {})
				);

				if (result)
				{
					return (result.cast<std::size_t>() > 0);
				}
			}
		}
		else
		{
			auto get_fn = type.func("get_component"_hs);

			if (!get_fn)
			{
				print_warn("Unable to resolve component-get function from: #{}", type.id());

				return false;
			}

			auto result = get_fn.invoke
			(
				{},

				entt::forward_as_meta(registry),
				entt::forward_as_meta(entity)
			);

			if (auto instance = *result) // ; (*result).cast<bool>()
			{
				if (value_assignment) // (&& direct_modify) <-- Already implied due to prior clause.
				{
					if (component.size() > 0)
					{
						const auto result = component.apply_fields(instance);

						//assert(result == component.size());

						if (result != component.size())
						{
							print_warn("Unable to apply all of the specified fields while updating component #{}", type.id());

							if (result == 0)
							{
								return false;
							}
						}

						if (auto notify_patch_fn = type.func("mark_component_as_patched"_hs))
						{
							auto notify_result = notify_patch_fn.invoke
							(
								{},

								entt::forward_as_meta(registry),
								entt::forward_as_meta(entity)
							);

							if (!notify_result)
							{
								print_warn("Failed to notify listeners of patch for component #{}", type.id());
							}
						}
						else
						{
							print_warn("Unable to resolve component-patch notification function from: #{}", type.id());
						}
					}
				}

				return true;
			}
		}

		return false;
	}
}