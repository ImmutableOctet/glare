#pragma once

#include <engine/types.hpp>
#include <engine/command.hpp>
#include <engine/meta/types.hpp>
#include <engine/meta/meta_type_descriptor.hpp>

namespace engine
{
	// Performs copy or move construction of `component` to `target` via an 'emplace-or-replace' operation.
	struct ComponentReplaceCommand : public Command
	{
		using Component = MetaAny;

		Component component;
	};
}