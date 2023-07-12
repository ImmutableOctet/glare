#pragma once

#include <engine/meta/types.hpp>
#include <engine/command.hpp>

namespace engine
{
	// Stores a component of the `source` entity.
	struct StoreComponentCommand : public Command
	{
		MetaTypeID component_type_id;
	};
}