#pragma once

#include <engine/reflection.hpp>

#include "commands/print_command.hpp"

namespace engine
{
    class DebugListener;

	template <>
    void reflect<PrintCommand>()
    {
        engine_command_type<PrintCommand>()
            .data<&PrintCommand::message>("message"_hs)
            .ctor<decltype(Command::source), decltype(Command::target), decltype(PrintCommand::message)>()
        ;
    }

    template <>
    void reflect<DebugListener>()
    {
        reflect<PrintCommand>();
    }
}