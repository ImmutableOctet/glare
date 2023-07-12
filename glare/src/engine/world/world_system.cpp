#include "world_system.hpp"
#include "world.hpp"

#include <engine/service_events.hpp>
#include <engine/service.hpp>

#include <cassert>

namespace engine
{
	WorldSystem::WorldSystem
	(
		World& world,
		
		bool allow_multiple_subscriptions,

		bool allow_update,
		bool allow_fixed_update,
		bool allow_render
	) :
		world(world),
		subscribed(false),
		allow_multiple_subscriptions(allow_multiple_subscriptions),
		allow_update(allow_update),
		allow_fixed_update(allow_fixed_update),
		allow_render(allow_render)
	{}

	WorldSystem::~WorldSystem()
	{
		// Unsubscribe from `world`.
		unsubscribe_impl(world, false);
	}

	void WorldSystem::update(const OnServiceUpdate& update_event)
	{
		//assert(allow_update);

		if (!allow_update)
		{
			return;
		}

		auto world_ptr = resolve_world(update_event);

		if (!world_ptr)
		{
			return;
		}

		on_update(*world_ptr, update_event.delta);
	}

	void WorldSystem::fixed_update(const OnServiceFixedUpdate& fixed_update_event)
	{
		//assert(allow_fixed_update);

		if (!allow_fixed_update)
		{
			return;
		}

		auto world_ptr = resolve_world(fixed_update_event);

		if (!world_ptr)
		{
			return;
		}

		on_fixed_update(*world_ptr, fixed_update_event.time); // fixed_update_event.delta
	}

	void WorldSystem::render(const OnServiceRender& render_event)
	{
		//assert(allow_render);

		if (!allow_render)
		{
			return;
		}

		auto world_ptr = resolve_world(render_event);

		if (!world_ptr)
		{
			return;
		}

		on_render(*world_ptr, *render_event.graphics);
	}

	void WorldSystem::on_render(World& world, app::Graphics& graphics) {}
	void WorldSystem::on_update(World& world, float delta) {}
	void WorldSystem::on_fixed_update(World& world, app::Milliseconds time) {}

	bool WorldSystem::subscribe(World& world)
	{
		bool is_same_as_internal = is_bound_world(world);

		if (is_same_as_internal && subscribed)
		{
			return false;
		}

		if (!is_same_as_internal && !allow_multiple_subscriptions)
		{
			return false;
		}

		//assert(&world == &this->world);

		if (allow_update)
		{
			world.register_event<OnServiceUpdate, &WorldSystem::update>(*this);
		}

		if (allow_fixed_update)
		{
			world.register_event<OnServiceFixedUpdate, &WorldSystem::fixed_update>(*this);
		}

		if (allow_render)
		{
			world.register_event<OnServiceRender, &WorldSystem::render>(*this);
		}

		on_subscribe(world);

		if (is_same_as_internal)
		{
			subscribed = true;
		}

		return true;
	}

	bool WorldSystem::unsubscribe(World& world)
	{
		return unsubscribe_impl(world, true);
	}

	bool WorldSystem::unsubscribe_impl(World& world, bool _dispatch)
	{
		bool is_same_as_internal = is_bound_world(world);

		if (is_same_as_internal && !subscribed)
		{
			return false;
		}

		if (!is_same_as_internal && !allow_multiple_subscriptions)
		{
			return false;
		}

		//assert(&world == &this->world);

		world.unsubscribe(*this);

		if (_dispatch)
		{
			on_unsubscribe(world);
		}

		if (is_same_as_internal)
		{
			subscribed = false;
		}

		return true;
	}

	World& WorldSystem::get_world() const
	{
		return world;
	}

	Registry& WorldSystem::get_registry() const
	{
		return world.get_registry();
	}

	bool WorldSystem::is_bound_world(Service* svc) const
	{
		return (svc == &this->world);
	}

	bool WorldSystem::is_bound_world(World* world) const
	{
		return (world == &this->world);
	}

	void WorldSystem::on_unsubscribe(World& world)
	{
		// Empty implementation; override with your own, if desired.
	}

	// Safely retrieves a `World` pointer from a base `Service` pointer.
	World* WorldSystem::resolve_world(Service* service)
	{
		if (service == &world)
		{
			return &world;
		}

		if (!allow_multiple_subscriptions)
		{
			return nullptr;
		}

		#if defined(WORLD_SYSTEM_ASSUME_SERVICE_IS_ALWAYS_WORLD) && (WORLD_SYSTEM_ASSUME_SERVICE_IS_ALWAYS_WORLD == 1)
			return reinterpret_cast<World*>(service);
		#else
			return dynamic_cast<World*>(service);
		#endif
	}

	template <typename ServiceEventType>
	World* WorldSystem::resolve_world(const ServiceEventType& event_obj)
	{
		return resolve_world(event_obj.service);
	}
}