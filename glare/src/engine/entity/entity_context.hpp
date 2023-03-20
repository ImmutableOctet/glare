#pragma once

#include "entity_variables.hpp"

#include <cstddef>

namespace engine
{
	struct EntityContext
	{
		std::size_t set_missing_variables(const EntityContext& existing_context);

		EntityVariables<16> variables; // 8
	};
}