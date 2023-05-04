#pragma once

#include <engine/meta/types.hpp>

#include <entt/meta/meta.hpp>

namespace engine
{
	// Retrieves a handle to the shared entt meta-context.
	entt::locator<entt::meta_ctx>::node_type get_shared_reflection_handle();

	// NOTE: This implementation is inline so that it is local to this module.
    // In contrast, the `get_shared_reflection_handle` function is unique between
    // modules (i.e. DLLs) since it's explicitly defined in a translation unit.
    inline void set_local_reflection_handle(const entt::locator<entt::meta_ctx>::node_type& handle)
    {
        entt::locator<entt::meta_ctx>::reset(handle);
    }

    // Synchronizes the reflection context across boundaries.
    // (Used for interop with dynamically linked modules)
    inline bool sync_reflection_context()
    {
        static bool is_synchronized = false;

        if (is_synchronized)
        {
            return true;
        }

        const auto& handle = get_shared_reflection_handle();

        set_local_reflection_handle(handle);

        is_synchronized = true;

        return is_synchronized;
    }
}