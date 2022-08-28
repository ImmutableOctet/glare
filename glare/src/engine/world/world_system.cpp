#include "world_system.hpp"
#include "world.hpp"

#include <engine/events/service_events.hpp>

#include <cassert>

namespace engine
{
	template <typename ServiceEventType>
	static World* resolve_world(const ServiceEventType& event_obj)
	{
		#if defined(WORLD_SYSTEM_ASSUME_SERVICE_IS_ALWAYS_WORLD) && (WORLD_SYSTEM_ASSUME_SERVICE_IS_ALWAYS_WORLD == 1)
			return reinterpret_cast<World*>(event_obj.service);
		#else
			return dynamic_cast<World*>(event_obj.service);
		#endif
	}

	void WorldSystem::update(const OnServiceUpdate& update_event)
	{
		auto* world = resolve_world(update_event);

		if (!world)
		{
			return;
		}

		on_update(*world, update_event.delta);
	}

	void WorldSystem::subscribe(World& world)
	{
		world.register_event<OnServiceUpdate, &WorldSystem::update>(*this);
	}

	void WorldSystem::unsubscribe(World& world)
	{
		world.unsubscribe(*this);

		on_unsubscribe(world);
	}

	void WorldSystem::on_unsubscribe(World& world)
	{
		// Empty implementation; override with your own if desired.
	}
}