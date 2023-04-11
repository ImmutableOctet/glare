#include "entity_factory.hpp"

#include "entity_construction_context.hpp"

//#include "serial_impl.hpp"

//#include "components/instance_component.hpp"
//#include "components/state_component.hpp"
//#include "components/entity_thread_component.hpp"

#include <engine/world/graphics_entity.hpp>

#include <engine/components/relationship_component.hpp>
#include <engine/components/name_component.hpp>

#include <engine/meta/types.hpp>
#include <engine/meta/hash.hpp>
#include <engine/meta/component.hpp>
//#include <engine/meta/function.hpp>

#include <engine/meta/meta_evaluation_context.hpp>

namespace engine
{
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

		auto evaluation_context = MetaEvaluationContext
		{
			.variable_context = {},
			.service          = context.opt_service,
			.system_manager   = context.opt_system_manager
		};

		for (const auto& component_entry : descriptor.components.type_definitions)
		{
			const auto& component = component_entry.get(descriptor);

			if (!component.has_indirection())
			{
				emplace_component(context.registry, entity, component, &evaluation_context);
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