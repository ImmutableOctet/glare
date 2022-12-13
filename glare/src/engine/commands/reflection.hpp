#pragma once

#include <engine/reflection.hpp>

#include "print_command.hpp"
#include "component_patch_command.hpp"
#include "component_replace_command.hpp"

namespace engine
{
	template <>
    void reflect<PrintCommand>()
    {
        engine_command_type<PrintCommand>()
            .base<Command>()
            .data<&PrintCommand::message>("message"_hs)
            .ctor<decltype(Command::source), decltype(Command::target), decltype(PrintCommand::message)>()
        ;
    }

	template <>
	void reflect<ComponentPatchCommand>()
	{
		engine_meta_type<ComponentPatchCommand>()
			.base<Command>()
			.data<&ComponentPatchCommand::component>("component"_hs)
			.ctor
			<
				decltype(ComponentPatchCommand::source),
				decltype(ComponentPatchCommand::target),
				decltype(ComponentPatchCommand::component)
			>()
		;
	}

	template <>
	void reflect<ComponentReplaceCommand>()
	{
		engine_meta_type<ComponentReplaceCommand>()
			.base<Command>()
			.data<&ComponentReplaceCommand::component>("component"_hs)
			.ctor
			<
				decltype(ComponentReplaceCommand::source),
				decltype(ComponentReplaceCommand::target),
				decltype(ComponentReplaceCommand::component)
			>()
		;
	}

	void reflect_core_commands()
	{
		reflect<PrintCommand>();
		reflect<ComponentPatchCommand>();
		reflect<ComponentReplaceCommand>();
	}
}