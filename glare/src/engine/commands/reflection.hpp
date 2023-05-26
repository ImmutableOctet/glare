#pragma once

#include <engine/reflection.hpp>

// TODO: Move this file into the `commands` submodule.
#include <engine/command.hpp>

#include "print_command.hpp"
#include "component_patch_command.hpp"
#include "indirect_component_patch_command.hpp"
#include "component_replace_command.hpp"
#include "function_command.hpp"
#include "expr_command.hpp"
#include "set_parent_command.hpp"

#include <engine/meta/meta_evaluation_context.hpp>

namespace engine
{
	template <>
    void reflect<Command>()
    {
        engine_meta_type
		<
			Command,

			MetaTypeReflectionConfig
			{
				.capture_standard_data_members = false
			}
		>()
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
	void reflect<IndirectComponentPatchCommand>()
	{
		engine_command_type<IndirectComponentPatchCommand>()
			.data<&IndirectComponentPatchCommand::component>("component"_hs)
			.data<&IndirectComponentPatchCommand::context>("context"_hs)
			.ctor
			<
				decltype(IndirectComponentPatchCommand::source),
				decltype(IndirectComponentPatchCommand::target),
				decltype(IndirectComponentPatchCommand::component)
			>()
			.ctor
			<
				decltype(IndirectComponentPatchCommand::source),
				decltype(IndirectComponentPatchCommand::target),
				decltype(IndirectComponentPatchCommand::component),
				decltype(IndirectComponentPatchCommand::context)
			>()
		;
	}

	template <>
	void reflect<ComponentPatchCommand>()
	{
		engine_command_type<ComponentPatchCommand>()
			.data<&ComponentPatchCommand::component>("component"_hs)
			.data<&ComponentPatchCommand::use_member_assignment>("use_member_assignment"_hs)
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
				decltype(ComponentPatchCommand::use_member_assignment)
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

	template <>
	void reflect<FunctionCommand>()
	{
		engine_command_type<FunctionCommand>()
			.data<&FunctionCommand::function>("function"_hs)
			.data<&FunctionCommand::context>("context"_hs)
		;
	}

	template <>
	void reflect<ExprCommand>()
	{
		engine_command_type<ExprCommand>()
			.data<&ExprCommand::expr>("expr"_hs)
			.data<&ExprCommand::context>("context"_hs)
		;
	}

	template <>
	void reflect<SetParentCommand>()
	{
		engine_command_type<SetParentCommand>()
			.data<&SetParentCommand::parent>("parent"_hs)
		;
	}

	void reflect_core_commands()
	{
		reflect<Command>();
		reflect<PrintCommand>();
		reflect<IndirectComponentPatchCommand>();
		reflect<ComponentPatchCommand>();
		reflect<ComponentReplaceCommand>();
		reflect<FunctionCommand>();
		reflect<ExprCommand>();
		reflect<SetParentCommand>();
	}
}