#pragma once

#include "delta_system_mode.hpp"

#include "types.hpp"

#include <engine/delta_time.hpp>
#include <engine/world/world_system.hpp>
#include <engine/meta/types.hpp>

#include <app/types.hpp>

namespace engine
{
	struct OnTransformChanged;

	class EntitySystem;
	class EntityListener;

	struct DeltaTrackerComponent;

	struct OnComponentCreate;
	struct OnComponentUpdate;
	struct OnComponentDestroy;

	class DeltaSystem : public WorldSystem
	{
		public:
			using Mode = DeltaSystemMode;

			DeltaSystem
			(
				World& world,
				EntitySystem& entity_system,

				DeltaTime::Rate ideal_rate,
				Mode mode=Mode::Default,

				bool only_track_tagged_entities=false
			);

			void snapshot(DeltaTimestamp timestamp);

			const DeltaTime& update_delta(app::Milliseconds time);

			inline const DeltaTime& get_delta_time() const { return delta_time; }
			inline DeltaTime& get_delta_time() { return delta_time; }

			float get_delta() const;
			DeltaTimestamp get_latest_delta_timestamp() const;

			inline operator float() const { return get_delta(); } // explicit
		
		private:
			EntitySystem& entity_system;

			std::size_t listen_for_components();

			void listen(bool components=true);

			EntityListener* listen(const MetaType& component_type);
			EntityListener* listen(MetaTypeID component_type_id);

		protected:

			void on_subscribe(World& world) override;
			void on_unsubscribe(World& world) override;

			void on_delta_tracker_listen(Entity entity, const DeltaTrackerComponent& tracker);
			void on_delta_tracker_listen(Registry& registry, Entity entity);

			void on_delta_tracker_create(Registry& registry, Entity entity);
			void on_delta_tracker_destroy(Registry& registry, Entity entity);
			void on_delta_tracker_update(Registry& registry, Entity entity);

			void on_update(World& world, float delta) override;
			void on_fixed_update(World& world, app::Milliseconds time) override;

			void on_transform_changed(const OnTransformChanged& tform_change);

			void on_component_create(const OnComponentCreate& component_details);
			void on_component_destroy(const OnComponentDestroy& component_details);
			void on_component_update(const OnComponentUpdate& component_details);
			
			bool is_tracked_component(Entity entity, MetaTypeID component_type_id) const;

			DeltaTime delta_time;

			DeltaSystemMode mode = DeltaSystemMode::Default;

			bool only_track_tagged_entities : 1 = false;
	};
}