#pragma once

#include <engine/types.hpp>
#include <engine/command.hpp>
#include <engine/meta/types.hpp>
#include <engine/meta/meta_type_descriptor.hpp>

namespace engine
{
	// Replaces the component-type described by `component` for `target`.
	struct ComponentReplaceCommand : public Command
	{
		using Component = MetaAny;

		Component component;
	};
}