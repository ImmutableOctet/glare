#include "entity_factory_data.hpp"
#include "resource_manager.hpp"

namespace engine
{
	Entity EntityFactoryData::create(const EntityConstructionContext& context, bool handle_children) const
	{
		if (handle_children && !children.empty())
		{
			// Allocate an initial (blank) entity to act as the parent
			// (See below for details):
			auto entity = context.registry.create();

			if (entity == null)
			{
				return null;
			}

			// Allocate and construct children:
			auto child_context = EntityConstructionContext
			{
				.registry = context.registry,
				.resource_manager = context.resource_manager,

				.parent = entity
			};

			// NOTE: Recursion.
			generate_children(child_context);

			// Finish constructing the entity we first allocated:
			auto self_context = context;

			self_context.opt_entity_out = entity;

			// NOTE: Construction of the initial entity is
			// deferred to allow referencing of its children.
			// (e.g. via nested `EntityTarget` objects in the descriptor)
			return factory.create(self_context);
		}
		
		// If there are no children present, construct a new entity immediately.
		return factory.create(context);
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

		for (const auto& child_path : children)
		{
			auto child_factory = resource_manager.get_existing_factory(child_path); // get_factory(child_path);

			// NOTE: May change this from an assert later.
			assert(child_factory);

			// TODO: Determine best fallback control path.
			if (!child_factory)
			{
				continue;
				//return false;
			}

			auto child = child_factory->create(child_context);

			if (child == null)
			{
				return false;
			}
		}

		return true;
	}
}