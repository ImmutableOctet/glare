#include "indirect_meta_variable_target.hpp"
#include "meta_evaluation_context.hpp"

#include <engine/entity/types.hpp>
#include <engine/entity/entity_system.hpp>
#include <engine/entity/entity_thread_range.hpp>
#include <engine/entity/entity_thread.hpp>
#include <engine/entity/entity_descriptor.hpp>
#include <engine/entity/components/entity_thread_component.hpp>
#include <engine/entity/components/instance_component.hpp>

#include <util/variant.hpp>

#include <optional>

namespace engine
{
	MetaAny IndirectMetaVariableTarget::get(const MetaVariableEvaluationContext& context) const
	{
		return variable.get(context);
	}

	MetaAny IndirectMetaVariableTarget::get(const MetaEvaluationContext& context) const
	{
		return variable.get(context);
	}

	MetaAny IndirectMetaVariableTarget::get(Registry& registry, Entity source) const
	{
		const auto target_entity = target.get(registry, source);

		auto variable_context = get_variable_context(registry, target_entity);

		return variable.get(registry, target_entity, MetaEvaluationContext { &variable_context });
	}

	MetaVariableEvaluationContext IndirectMetaVariableTarget::get_variable_context(Registry& registry, Entity entity) const
	{
		std::optional<EntityThreadIndex> thread_index = std::nullopt;

		auto* thread_comp = registry.try_get<EntityThreadComponent>(entity);

		EntityThread* thread_instance = {};

		util::visit
		(
			thread.value,

			[](const EntityThreadTarget::Empty&) {},

			[&thread_comp, &thread_index, &thread_instance](const EntityThreadRange& thread_range)
			{
				assert(thread_range.size() == 1);

				thread_index = thread_range.start_index;

				if (thread_comp)
				{
					thread_instance = thread_comp->get_thread(*thread_index);
				}
			},

			[&registry, entity, &thread_comp, &thread_index, &thread_instance](EntityThreadID thread_id)
			{
				if (const auto* instance_comp = registry.try_get<InstanceComponent>(entity))
				{
					auto& descriptor = instance_comp->get_descriptor();

					if (thread_comp)
					{
						thread_instance = thread_comp->get_thread(descriptor, thread_id);
					}

					if (thread_instance)
					{
						thread_index = thread_instance->thread_index;
					}
					else
					{
						thread_index = descriptor.get_thread_index(thread_id);
					}
				}
			}
		);

		return EntitySystem::resolve_variable_context
		(
			nullptr,
			&registry,
			entity,
			thread_comp,
			thread_instance
		);
	}

	IndirectMetaVariableTarget& IndirectMetaVariableTarget::set(MetaAny& value, MetaVariableEvaluationContext& context)
	{
		variable.set(value, context);

		return *this;
	}

	IndirectMetaVariableTarget& IndirectMetaVariableTarget::set(MetaAny& value, const MetaEvaluationContext& context)
	{
		variable.set(value, context);

		return *this;
	}

	IndirectMetaVariableTarget& IndirectMetaVariableTarget::set(MetaAny& value, Registry& registry, Entity source)
	{
		const auto target_entity = target.get(registry, source);

		auto variable_context = get_variable_context(registry, target_entity);

		variable.set(value, registry, target_entity, MetaEvaluationContext { &variable_context });

		return *this;
	}
}