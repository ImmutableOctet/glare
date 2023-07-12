#pragma once

#include "history_log.hpp"

#include <engine/basic_system.hpp>

namespace engine
{
	struct UndoCommand;
	struct RedoCommand;
	struct StoreSnapshotCommand;

	struct OnDeltaSnapshot;

	struct OnDeltaLifetimeEnd;
	struct OnDeltaLifetimeRestart;

	class HistorySystem : public BasicSystem
	{
		public:
			HistorySystem(Service& service);

			const HistoryLog& get_history_log() const;
			HistoryLog& get_history_log();

		protected:
			bool on_subscribe(Service& service) override;

			void on_delta_lifetime_end(const OnDeltaLifetimeEnd& lifetime_details);
			void on_delta_lifetime_restart(const OnDeltaLifetimeRestart& lifetime_details);

			void on_delta_snapshot(const OnDeltaSnapshot& delta_snapshot);

			void on_undo_command(const UndoCommand& undo_command);
			void on_redo_command(const RedoCommand& redo_command);
			void on_store_snapshot_command(const StoreSnapshotCommand& snapshot_command);

			HistoryLog history;

		private:
			MetaAny store_component(Registry& registry, Entity entity, const MetaType& component_type, bool allow_emplace=true, bool update_cursor=true);
			MetaAny store_component(Registry& registry, Entity entity, MetaTypeID component_type_id, bool allow_emplace=true, bool update_cursor=true);

			MetaAny store_back_component(Registry& registry, Entity entity, const MetaType& component_type);
			MetaAny store_back_component(Registry& registry, Entity entity, MetaTypeID component_type_id);

			MetaAny remove_latest_component_store(Registry& registry, Entity entity, const MetaType& component_type);
			MetaAny remove_latest_component_store(Registry& registry, Entity entity, MetaTypeID component_type_id);
	};
}