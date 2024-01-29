#pragma once

#include "empty.hpp"

namespace engine
{
	template <typename T>
    auto engine_system_type(bool sync_context=true)
    {
        return engine_empty_type<T>(sync_context)
            .prop("system"_hs)
        ;
    }
}