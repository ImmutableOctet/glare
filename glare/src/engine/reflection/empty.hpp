#pragma once

#include "context.hpp"

#include <engine/meta/meta.hpp>
#include <engine/meta/short_name.hpp>

namespace engine
{
	namespace impl
	{
		template <typename T>
        auto engine_empty_meta_type()
        {
            auto type = entt::meta<T>()
                .type(short_name_hash<T>())
            ;

            return type;
        }
	}

    template <typename T>
    auto engine_empty_type(bool sync_context=true)
    {
        if (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        return impl::engine_empty_meta_type<T>();
    }
}