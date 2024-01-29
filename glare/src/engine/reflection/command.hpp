#pragma once

#include "core.hpp"

#include <engine/command.hpp>

namespace engine
{
    template <typename T>
    auto engine_command_type(bool sync_context = true)
    {
        return engine_meta_type
        <
            T,

            MetaTypeReflectionConfig
            {
                .capture_standard_data_members = false
            }
        > (sync_context)
            .base<Command>()
            //.prop("command"_hs)
        ;
    }
}