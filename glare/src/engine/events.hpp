#pragma once

/*
	Standard game engine event types.

	System-specific event types can usually be found
	in `events` submodules for each system.
*/

#include "types.hpp"

//#include <app/input/events.hpp>

namespace engine
{
	// Input event types:
	//using app::input::OnGamepadConnected;
	//using app::input::OnGamepadDisconnected;

	// A notification that an entity has been created.
	// This notification is deferred until the next update/event-dispatch after the creation took place.
	// For immediate events, please see entt's `basic_registry::on_construct` event sink(s).
	struct OnEntityCreated
	{
		//static constexpr auto Type = type;

		inline EntityType get_type() const { return type; }

		Entity entity;
		Entity parent;
		EntityType type;
	};

	// A notification that `entity` has been destroyed.
	// This is used internally by `World` to signal a deferred destruction of an entity.
	// Full destruction on the entt side does not take place until this message is received by a `World` object.
	// For an 'immediate' destruction event (i.e. when entt disposes of the entity), see the `basic_registry::on_destroy` event sink(s).
	struct OnEntityDestroyed
	{
		//static constexpr auto Type = type;
		inline EntityType get_type() const { return type; }

		Entity entity;
		Entity parent;
		EntityType type;

		bool destroy_orphans = true;
	};

	// This is triggered immediately when the parent of an entity has changed.
	struct OnParentChanged
	{
		Entity entity, from_parent, to_parent;
	};
}