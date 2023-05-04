#include "context.hpp"

namespace engine
{
	entt::locator<entt::meta_ctx>::node_type get_shared_reflection_handle()
    {
        return entt::locator<entt::meta_ctx>::handle();
    }
}