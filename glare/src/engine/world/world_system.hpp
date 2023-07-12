#pragma once

#include <engine/types.hpp>
//#include <engine/basic_system.hpp>

#include <app/types.hpp>

// If this macro is defined, we will always assume an inbound service-event is coming from a `World` object.
// This can act as a minor speedup as we won't have to use RTTI (`dynamic_cast`) in order to determine the exact `Service` type.
//#define WORLD_SYSTEM_ASSUME_SERVICE_IS_ALWAYS_WORLD 1

namespace app
{
	struct Graphics;
}

namespace engine
{
	// Forward declaractions:
	class World;
	class Service;

	// Event types:
	struct OnServiceUpdate;
	struct OnServiceFixedUpdate;
	struct OnServiceRender;

	// Utility class for systems within a `World`.
	class WorldSystem // : public BasicSystemImpl<World>
	{
		public:
			// NOTE: Subscription is deferred until the system is fully constructed.
			// See also: `SystemManager`.
			WorldSystem
			(
				World& world,

				bool allow_multiple_subscriptions=false,

				bool allow_update=true,
				bool allow_fixed_update=true,
				bool allow_render=true
			);

			virtual ~WorldSystem();

			//WorldSystem(WorldSystem&&) = delete;
			//WorldSystem(const WorldSystem&) = delete;

			/*
				NOTES:
				
				It is safe to call this method multiple times for the internal `world` object.
				
				If `allow_multiple_subscriptions` is true, it is valid to call this method with a different
				`World` object than `WorldSystem::world`, as long as no more than one concurrent subscription is active.

				Multiple concurrent subscriptions to the same service are considered undefined behavior.
			*/
			bool subscribe(World& world);

			// See notes for `subscribe`.
			bool unsubscribe(World& world);

			// Retrieves the internal `World` object this service was created with.
			// NOTE: This `World` object may be 'only symbolically linked' if a subscription has not taken place.
			// To enasure that subscription has happened successfully, please check against `is_subscribed`.
			World& get_world() const;

			// Retrieves a registry from `world`.
			// Equivalent to: `get_world().get_registry()`
			Registry& get_registry() const;

			// Indicates whether a subscription has been established with the internal `World` object.
			// This does not report when other services have subscriptions with this system.
			inline bool is_subscribed() const { return subscribed; }

			// Returns true if `svc` points to the same object as `WorldSystem::world`.
			bool is_bound_world(Service* svc) const;

			// Returns true if `world` is the same as `WorldSystem::world`.
			bool is_bound_world(World* world) const;

			// Returns true if `svc` is the same as `WorldSystem::world`.
			inline bool is_bound_world(Service& svc) const { return is_bound_world(&svc); }

			// Returns true if `world` points to the same object as `WorldSystem::world`.
			inline bool is_bound_world(World& world) const { return is_bound_world(&world); }
		private:
			// Used internally. (Declared here, defined in source file)
			template <typename ServiceEventType>
			World* resolve_world(const ServiceEventType& event_obj);

			void update(const OnServiceUpdate& update_event);
			void fixed_update(const OnServiceFixedUpdate& fixed_update_event);
			void render(const OnServiceRender& render_event);

			// Implementation of `unsubscribe`. (Used internally)
			bool unsubscribe_impl(World& world, bool _dispatch=true);
		protected:
			// Safely retrieves a `World` pointer from a base `Service` pointer.
			// NOTE: If `allow_multiple_subscriptions` is false, this will only
			// return non-null when `WorldSystem::world` is supplied.
			World* resolve_world(Service* service);

			// Subscribes to the internally stored `world` object.
			inline bool subscribe() { return subscribe(world); }

			// Unsubscribes from the internally stored `world` object.
			inline bool unsubscribe() { return unsubscribe(world); }

			// Called by `subscribe` after regular subscription actions are performed.
			virtual void on_subscribe(World& world) = 0;
			
			// Default implementation; blank.
			virtual void on_update(World& world, float delta);

			// Default implementation; blank.
			virtual void on_fixed_update(World& world, app::Milliseconds time);

			// Default implementation; blank.
			virtual void on_render(World& world, app::Graphics& graphics);

			// Empty implementation provided by default.
			// 
			// NOTE:
			// This is not called if the unsubscription
			// was performed during destruction.
			virtual void on_unsubscribe(World& world);

			// The `World` instance this system is linked to.
			World& world;

			// Flags:

			// Whether we're subscribed to `world`.
			bool subscribed                   : 1;

			// Whether we allow for multiple subscriptions concurrently.
			bool allow_multiple_subscriptions : 1;

			bool allow_update                 : 1;
			bool allow_fixed_update           : 1;
			bool allow_render                 : 1;
	};
}