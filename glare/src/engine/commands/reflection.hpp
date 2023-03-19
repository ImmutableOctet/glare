#pragma once

#include <engine/reflection.hpp>

// TODO: Move this file into the `commands` submodule.
#include <engine/command.hpp>

#include "print_command.hpp"
#include "component_patch_command.hpp"
#include "component_replace_command.hpp"

#include <engine/meta/meta_evaluation_context.hpp>

namespace engine
{
	template <>
    void reflect<Command>()
    {
        engine_meta_type<Command>(false)
			//.data<nullptr, &Command::entity>("entity"_hs)
            .data<&Command::source>("source"_hs)
            .data<&Command::target>("target"_hs)
            //.ctor<decltype(Command::source), decltype(Command::target)>()
        ;
    }

	template <>
    void reflect<PrintCommand>()
    {
        engine_command_type<PrintCommand>()
            .data<&PrintCommand::message>("message"_hs)
            .ctor<decltype(Command::source), decltype(Command::target), decltype(PrintCommand::message)>()
        ;
    }

	template <>
	void reflect<ComponentPatchCommand>()
	{
		engine_command_type<ComponentPatchCommand>()
			.data<&ComponentPatchCommand::component>("component"_hs)
			.data<&ComponentPatchCommand::context>("context"_hs)
			.ctor
			<
				decltype(ComponentPatchCommand::source),
				decltype(ComponentPatchCommand::target),
				decltype(ComponentPatchCommand::component)
			>()
			.ctor
			<
				decltype(ComponentPatchCommand::source),
				decltype(ComponentPatchCommand::target),
				decltype(ComponentPatchCommand::component),
				decltype(ComponentPatchCommand::context)
			>()
		;
	}

	template <>
	void reflect<ComponentReplaceCommand>()
	{
		engine_command_type<ComponentReplaceCommand>()
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
		reflect<Command>();
		reflect<PrintCommand>();
		reflect<ComponentPatchCommand>();
		reflect<ComponentReplaceCommand>();
	}
}