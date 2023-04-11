#include "entity_factory_data.hpp"
#include "resource_manager.hpp"

#include <engine/entity/entity_construction_context.hpp>
#include <engine/entity/components/instance_component.hpp>
#include <engine/entity/components/entity_thread_component.hpp>

#include <engine/meta/component.hpp>
#include <engine/meta/meta_evaluation_context.hpp>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	Entity EntityFactoryData::create(std::shared_ptr<const EntityFactoryData> factory_data, const EntityConstructionContext& context, bool handle_children)
	{
		if (!factory_data)
		{
			return null;
		}

		const auto* factory_data_raw_ptr = factory_data.get();

		return factory_data_raw_ptr->create_impl(std::move(factory_data), context, handle_children);
	}

	Entity EntityFactoryData::create_impl(std::shared_ptr<const EntityFactoryData>&& factory_data, const EntityConstructionContext& context, bool handle_children) const
	{
		auto entity = factory.create(context);

		if (entity == null)
		{
			return null;
		}

		context.registry.emplace_or_replace<InstanceComponent>(entity, std::move(factory_data));

		if (handle_children && !children.empty())
		{
			// Allocate and construct children:
			auto child_context = EntityConstructionContext
			{
				.registry           = context.registry,
				.resource_manager   = context.resource_manager,

				.parent             = entity,

				.opt_entity_out     = null,

				.opt_service        = context.opt_service,
				.opt_system_manager = context.opt_system_manager
			};

			// NOTE: Recursion.
			generate_children(child_context);

			// NOTE: Construction of the initial entity is
			// deferred to allow referencing of its children.
			// (e.g. via nested `EntityTarget` objects in the descriptor)
		}

		return on_entity_create(entity, context);
	}

	Entity EntityFactoryData::on_entity_create(Entity entity, const EntityConstructionContext& context) const
	{
		const auto& descriptor = factory.get_descriptor();
		const auto& default_state_index = factory.get_default_state_index();

		auto evaluation_context = MetaEvaluationContext
		{
			.variable_context = {},
			.service          = context.opt_service,
			.system_manager   = context.opt_system_manager
		};

		// Instantiate remaining (indirection-dependent) components:
		for (const auto& component_entry : descriptor.components.type_definitions)
		{
			const auto& component = component_entry.get(descriptor);

			if (component.has_indirection())
			{
				emplace_component(context.registry, entity, component, &evaluation_context);
			}
		}

		if (default_state_index)
		{
			auto result = descriptor.set_state_by_index(context.registry, entity, std::nullopt, *default_state_index);

			if (!result)
			{
				print_warn("Unable to assign default state: Index #{}", *default_state_index);
			}
		}

		if (!descriptor.immediate_threads.empty())
		{
			auto& thread_component = context.registry.get_or_emplace<EntityThreadComponent>(entity);

			for (const auto& thread_range : descriptor.immediate_threads)
			{
				thread_component.start_threads(thread_range);
			}

			// Trigger listeners looking for `EntityThreadComponent` patches.
			context.registry.patch<EntityThreadComponent>(entity);
		}

		return entity;
	}

	// NOTE: Recursion via inner calls to `create` and `generate_children`.
	bool EntityFactoryData::generate_children(const EntityConstructionContext& context, Entity parent) const
	{
		if (parent != null)
		{
			EntityConstructionContext temp_context = context;

			temp_context.parent = parent;

			return generate_children(temp_context);
		}

		return generate_children(context);
	}

	bool EntityFactoryData::generate_children(const EntityConstructionContext& child_context) const
	{
		auto& resource_manager = child_context.resource_manager;

		for (const auto& child_factory_data : children)
		{
			// NOTE: May change this from an assert later.
			assert(child_factory_data);

			// TODO: Determine best fallback control path.
			if (!child_factory_data)
			{
				continue;
				//return false;
			}

			// NOTE: Recursion.
			create(std::move(child_factory_data), child_context, true);
		}

		return true;
	}
}