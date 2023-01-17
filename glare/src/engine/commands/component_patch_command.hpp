#pragma once

#include <engine/types.hpp>
#include <engine/command.hpp>
//#include <engine/meta/types.hpp>
//#include <engine/meta/meta_type_descriptor.hpp>

namespace engine
{
	struct MetaTypeDescriptor;

	// Updates the component-type described by `component` for `target`.
	struct ComponentPatchCommand : public Command
	{
		using ComponentChanges = const MetaTypeDescriptor*;

		ComponentChanges component;
	};
}