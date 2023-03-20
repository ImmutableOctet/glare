#pragma once

//#include "types.hpp"

#include <memory>

namespace engine
{
	struct EntityContext;

	struct EntityContextComponent
	{
		std::size_t adopt(const std::shared_ptr<EntityContext>& context_in);
		std::size_t adopt(EntityContext& context_in);

		// NOTE: Context allocation and management is handled automatically via `EntitySystem`.
		// Simply attach `EntityContextComponent` to start using this functionality.
		std::shared_ptr<EntityContext> shared_context;
	};
}