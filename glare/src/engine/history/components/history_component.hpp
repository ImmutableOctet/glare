#pragma once

#include <engine/types.hpp>

#include <util/history_log.hpp>

namespace engine
{
	template <typename ComponentType>
	struct HistoryComponent
	{
		using LogType = util::HistoryLog<ComponentType>;

		bool store(const ComponentType& component_instance)
		{
			return history.store(component_instance);
		}

		bool store(Registry& registry, Entity entity)
		{
			if (const auto component_instance = registry.try_get<ComponentType>(entity))
			{
				return store(*component_instance);
			}

			return false;
		}

		bool store_back(const ComponentType& component_instance)
		{
			return history.store_back(component_instance);
		}

		bool store_back(Registry& registry, Entity entity)
		{
			if (const auto component_instance = registry.try_get<ComponentType>(entity))
			{
				return store_back(*component_instance);
			}

			return false;
		}

		bool truncate()
		{
			return history.truncate();
		}

		bool truncate_back()
		{
			return truncate_back(false);
		}

		bool truncate_back(bool update_cursor)
		{
			return history.truncate_back(update_cursor);
		}

		bool undo(ComponentType& component_instance)
		{
			return history.undo
			(
				component_instance,
				
				[&component_instance](auto& history) -> bool
				{
					// Store `component_instance` at the end of the log, without adjusting the cursor.
					return history.store_back(component_instance);
				}
			);
		}

		bool undo(Registry& registry, Entity entity)
		{
			if (auto component_instance = registry.try_get<ComponentType>(entity))
			{
				if (undo(*component_instance))
				{
					// Mark the underlying component as patched.
					registry.patch<ComponentType>(entity);

					return true;
				}
			}

			return false;
		}

		bool redo(ComponentType& component_instance)
		{
			return history.redo(component_instance);
		}

		bool redo(Registry& registry, Entity entity)
		{
			if (auto component_instance = registry.try_get<ComponentType>(entity))
			{
				if (redo(*component_instance))
				{
					// Mark the underlying component as patched.
					registry.patch<ComponentType>(entity);

					return true;
				}
			}

			return false;
		}

		bool cursor_out_of_bounds(std::size_t cursor) const
		{
			return history.cursor_out_of_bounds(static_cast<typename LogType::SnapshotCursor>(cursor));
		}

		const ComponentType* get_snapshot(std::size_t index) const
		{
			return history.get_snapshot(static_cast<typename LogType::size_type>(index));
		}

		const ComponentType* get_active_snapshot() const
		{
			return history.get_active_snapshot();
		}

		std::size_t get_cursor() const
		{
			return static_cast<std::size_t>(history.get_cursor());
		}

		std::size_t get_live_value_cursor() const
		{
			return static_cast<std::size_t>(history.live_value_cursor());
		}

		std::size_t get_default_cursor() const
		{
			return static_cast<std::size_t>(LogType::default_cursor());
		}

		std::size_t size() const
		{
			return static_cast<std::size_t>(history.size());
		}

		bool empty() const
		{
			return history.empty();
		}

		bool can_undo() const
		{
			return history.can_undo();
		}

		bool can_redo() const
		{
			return history.can_redo();
		}

		bool can_truncate() const
		{
			return history.can_truncate();
		}

		bool can_store() const
		{
			return history.can_store();
		}

		bool can_clear() const
		{
			return history.can_clear();
		}

		bool can_copy_value() const
		{
			return history.can_copy_value();
		}

		bool can_move_value() const
		{
			return history.can_move_value();
		}

		bool has_default_cursor() const
		{
			return history.has_default_cursor();
		}

		bool has_live_value_cursor() const
		{
			return history.has_live_value_cursor();
		}

		LogType history;
	};
}