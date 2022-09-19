#pragma once

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

	// Event types:
	struct OnServiceUpdate;
	struct OnServiceRender;

	// Utility class for systems within a `World`.
	class WorldSystem
	{
		public:
			virtual ~WorldSystem();

			void subscribe(World& world);
			void unsubscribe(World& world);
		private:
			void update(const OnServiceUpdate& update_event);
			void render(const OnServiceRender& render_event);
		protected:
			// Called by `subscribe` after default subscription actions are performed.
			virtual void on_subscribe(World& world) = 0;
			virtual void on_update(World& world, float delta) = 0;

			// Default implementation; blank.
			virtual void on_render(World& world, app::Graphics& graphics);

			// Empty implementation provided by default.
			virtual void on_unsubscribe(World& world);
	};
}