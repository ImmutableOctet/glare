#pragma once

#include "types.hpp"

#include <engine/entity_type.hpp>

namespace engine
{
	struct TypeComponent
	{
		EntityType type = EntityType::Default;
	};
}