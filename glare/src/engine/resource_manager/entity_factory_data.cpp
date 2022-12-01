#include "entity_factory_data.hpp"
#include "resource_manager.hpp"

namespace engine
{
	Entity EntityFactoryData::create(const EntityConstructionContext& context) const
	{
		auto entity = factory.create(context);

		auto child_context = EntityConstructionContext
		{
			.registry = context.registry,
			.resource_manager = context.resource_manager,

			.parent = entity
		};

		auto& resource_manager = child_context.resource_manager;

		for (const auto& child_path : children)
		{
			auto child_factory = resource_manager.get_existing_factory(child_path); // get_factory(child_path);

			assert(child_factory);

			if (!child_factory)
			{
				continue;
			}

			child_factory->create(child_context);
		}

		return entity;
	}
}