#pragma once

#include "empty.hpp"

namespace engine
{
	template <typename T>
    auto engine_static_type(bool sync_context=true)
    {
        return engine_empty_type<T>(sync_context)
            .prop("static"_hs)
        ;
    }

    template <typename T>
    auto engine_global_static_type(bool sync_context=true)
    {
        return engine_static_type<T>(sync_context)
            .prop("global namespace"_hs)
        ;
    }
}