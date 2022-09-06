#include "meta.hpp"

// Full definitions of `Service` and `World` needed for `dynamic_cast` usage.
//#include <engine/service.hpp>
#include <engine/world/world.hpp>

namespace engine
{
	namespace behavior_impl
	{
		// Const version.
		const World* resolve_world_from_service(const Service* service)
		{
			#if defined(BEHAVIOR_ASSUME_SERVICE_IS_ALWAYS_WORLD) && (BEHAVIOR_ASSUME_SERVICE_IS_ALWAYS_WORLD == 1)
				return reinterpret_cast<const World*>(service);
			#else
				return dynamic_cast<const World*>(service);
			#endif
		}

		// Non-const version.
		World* resolve_world_from_service(Service* service)
		{
			#if defined(BEHAVIOR_ASSUME_SERVICE_IS_ALWAYS_WORLD) && (BEHAVIOR_ASSUME_SERVICE_IS_ALWAYS_WORLD == 1)
				return reinterpret_cast<World*>(service);
			#else
				return dynamic_cast<World*>(service);
			#endif
		}
	}
}