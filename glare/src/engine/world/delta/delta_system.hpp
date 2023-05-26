#pragma once

#include <engine/delta_time.hpp>
#include <engine/world/world_system.hpp>

#include <app/types.hpp>

namespace engine
{
	struct OnTransformChanged;

	class DeltaSystem : public WorldSystem
	{
		public:
			DeltaSystem(World& world, DeltaTime::Rate ideal_rate);

			const DeltaTime& update_delta(app::Milliseconds time);

			inline const DeltaTime& get_delta_time() const { return delta_time; }
			inline DeltaTime& get_delta_time() { return delta_time; }

			float get_delta() const;

			inline operator float() const { return get_delta(); } // explicit

		protected:
			void on_subscribe(World& world) override;
			void on_unsubscribe(World& world) override;

			void on_update(World& world, float delta) override;

			void on_transform_changed(const OnTransformChanged& tform_change);

			DeltaTime delta_time;
	};
}