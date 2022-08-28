#pragma once

// If this macro is defined, we will always assume an inbound service-event is coming from a `World` object.
// This can act as a minor speedup as we won't have to use RTTI (`dynamic_cast`) in order to determine the exact `Service` type.
//#define WORLD_SYSTEM_ASSUME_SERVICE_IS_ALWAYS_WORLD 1

namespace engine
{
	// Forward declaractions:
	class World;

	// Event types:
	struct OnServiceUpdate;

	// Utility class for systems within a `World`.
	class WorldSystem
	{
		public:
			void subscribe(World& world);
			void unsubscribe(World& world);
		private:
			void update(const OnServiceUpdate& update_event);
		protected:
			// Called by `subscribe` after default subscription actions are performed.
			virtual void on_subscribe(World& world) = 0;
			virtual void on_unsubscribe(World& world);
			virtual void on_update(World& world, float delta) = 0;
	};
}