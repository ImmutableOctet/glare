#include "entity_factory.hpp"

#include "entity_construction_context.hpp"

//#include "serial_impl.hpp"

//#include "components/instance_component.hpp"
//#include "components/state_component.hpp"
//#include "components/entity_thread_component.hpp"

#include <engine/world/graphics_entity.hpp>

#include <engine/components/relationship_component.hpp>
#include <engine/components/name_component.hpp>

#include <engine/meta/meta.hpp>
#include <engine/meta/meta_evaluation_context.hpp>

namespace engine
{
	MetaAny EntityFactory::emplace_component
	(
		Registry& registry, Entity entity,
		const MetaTypeDescriptor& component
	)
	{
		using namespace engine::literals;

		auto instance = component.instance(true, registry, entity);

		//assert(instance);

		MetaAny result;

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

	bool EntityFactory::update_component_fields
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

	Entity EntityFactory::create(const EntityConstructionContext& context) const
	{
		auto entity = context.opt_entity_out;

		if (entity == null)
		{
			entity = context.registry.create();
		}

		if (context.parent != null)
		{
			RelationshipComponent::set_parent(context.registry, entity, context.parent, true);
		}

		for (const auto& component_entry : descriptor.components.type_definitions)
		{
			const auto& component = component_entry.get(descriptor);

			if (!component.has_indirection())
			{
				emplace_component(context.registry, entity, component);
			}
		}

		if (!context.registry.try_get<NameComponent>(entity))
		{
			auto instance_filename = paths.instance_path.filename(); instance_filename.replace_extension();

			context.registry.emplace<NameComponent>(entity, instance_filename.string());
		}

		return entity;
	}
}