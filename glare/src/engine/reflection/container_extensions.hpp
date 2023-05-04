#pragma once

#include <engine/meta/types.hpp>

//#include <entt/meta/meta.hpp>

namespace engine::impl
{
	inline entt::meta_sequence_container::iterator meta_sequence_container_push_back_impl(entt::meta_sequence_container& container, MetaAny value) // const MetaAny&
    {
        /*
        // Alternative implementation:
        auto it = (container.begin());

        for (std::size_t i = 0; i < (container.size() - 1); i++)
        {
            it = it++;
        }
        */

        /*
        // Alternative implementation:
        auto it = (container.begin());

        const auto current_container_size = container.size();

        if (current_container_size > 0)
        {
            it.operator++(static_cast<int>(current_container_size - 1));
        }
        */

        auto it = container.end();

        return container.insert(it, std::move(value));
    }

    inline entt::meta_sequence_container::iterator meta_sequence_container_at_impl(entt::meta_sequence_container& container, std::int32_t index) // std::size_t // const
    {
        auto it = container.begin();

        if (index > 0)
        {
            it.operator++((index - 1));
        }

        return it;
    }

    inline entt::meta_sequence_container::iterator meta_sequence_container_find_impl(entt::meta_sequence_container& container, const MetaAny& value) // const
    {
        return std::find(container.begin(), container.end(), value);
    }
}