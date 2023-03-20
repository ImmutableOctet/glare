#include "entity_context_component.hpp"

#include <engine/entity/entity_context.hpp>

namespace engine
{
	std::size_t EntityContextComponent::adopt(const std::shared_ptr<EntityContext>& context_in)
	{
		if (!context_in)
		{
			return 0;
		}

		return adopt(*context_in);
	}

	std::size_t EntityContextComponent::adopt(EntityContext& context_in)
	{
		const auto variables_adopted = this->shared_context->set_missing_variables(context_in);

		return variables_adopted;
	}
}