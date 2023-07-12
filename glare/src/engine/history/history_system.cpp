#include "history_system.hpp"

#include "actions/action.hpp"

#include "commands/undo_command.hpp"
#include "commands/redo_command.hpp"
#include "commands/store_snapshot_command.hpp"

#include <engine/meta/types.hpp>
#include <engine/meta/component.hpp>
#include <engine/meta/events.hpp>

#include <engine/world/delta/events.hpp>
#include <engine/world/delta/components/delta_component.hpp>
#include <engine/world/delta/components/lifetime_delta_component.hpp>

#include <engine/service.hpp>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	HistorySystem::HistorySystem(Service& service)
		: BasicSystem(service)
	{}

	bool HistorySystem::on_subscribe(Service& service)
	{
		//auto& registry = service.get_registry();
		// ...

		service.register_event<OnDeltaLifetimeEnd, &HistorySystem::on_delta_lifetime_end>(*this);
		service.register_event<OnDeltaLifetimeRestart, &HistorySystem::on_delta_lifetime_restart>(*this);

		service.register_event<OnDeltaSnapshot, &HistorySystem::on_delta_snapshot>(*this);

		return true;
	}

	const HistoryLog& HistorySystem::get_history_log() const
	{
		return history;
	}

	HistoryLog& HistorySystem::get_history_log()
	{
		return history;
	}

	void HistorySystem::on_delta_lifetime_end(const OnDeltaLifetimeEnd& lifetime_details)
	{
		const auto component_type = resolve(lifetime_details.component_type_id);

		if (!component_type)
		{
			return;
		}

		auto& registry = get_registry();
		const auto& entity = lifetime_details.entity;

		// Store the final state of the component.
		store_back_component(registry, entity, component_type);
	}

	void HistorySystem::on_delta_lifetime_restart(const OnDeltaLifetimeRestart& lifetime_details)
	{
		const auto component_type = resolve(lifetime_details.component_type_id);

		if (!component_type)
		{
			return;
		}

		auto& registry = get_registry();
		const auto& entity = lifetime_details.entity;

		remove_latest_component_store(registry, entity, component_type);
	}

	void HistorySystem::on_delta_snapshot(const OnDeltaSnapshot& delta_snapshot)
	{
		using namespace engine::literals;

		auto& registry = get_registry();

		auto snapshot = HistoryEntry { delta_snapshot.snapshot_timestamp };

		registry.view<DeltaComponent>().each
		(
			[this, &registry, &snapshot](Entity entity, const DeltaComponent& delta_comp)
			{
				if (!delta_comp.modified_components.empty())
				{
					auto modifications = history::ComponentModifications { entity };

					for (const auto& component_id : delta_comp.modified_components)
					{
						if (auto history_comp = store_component(registry, entity, component_id))
						{
							const auto history_component_type = history_comp.type();

							assert(history_component_type);

							const auto history_component_id = history_component_type.id();

							assert(history_component_id);

							modifications.components_modified.emplace_back(history_component_id); // component_id
						}
					}

					snapshot.actions.emplace_back(std::move(modifications));
				}
			}
		);

		registry.view<LifetimeDeltaComponent>().each
		(
			[this, &registry, &snapshot](Entity entity, const LifetimeDeltaComponent& delta_comp)
			{
				if (!delta_comp.created_components.empty())
				{
					auto creations = history::ComponentCreations { entity };

					for (const auto& component_id : delta_comp.created_components)
					{
						const auto component_type = resolve(component_id);

						if (!component_type)
						{
							continue;
						}

						if (auto history_comp = store_component(registry, entity, component_type))
						{
							const auto history_component_type = history_comp.type();

							assert(history_component_type);

							const auto history_component_id = history_component_type.id();

							assert(history_component_id);

							creations.components_created.emplace_back(history_component_id); // component_id
						}
					}

					snapshot.actions.emplace_back(std::move(creations));
				}

				if (!delta_comp.destroyed_components.empty())
				{
					auto destructions = history::ComponentDestructions { entity };

					for (const auto& component_id : delta_comp.destroyed_components)
					{
						if (auto history_comp = get_history_component_ref(registry, entity, component_id))
						{
							const auto history_component_type = history_comp.type();

							assert(history_component_type);

							const auto history_component_id = history_component_type.id();

							assert(history_component_id);

							destructions.components_destroyed.emplace_back(history_component_id); // component_id
						}
					}

					snapshot.actions.emplace_back(std::move(destructions));
				}
			}
		);

		if (snapshot)
		{
			history.store(std::move(snapshot));
		}
	}

	void HistorySystem::on_undo_command(const UndoCommand& undo_command)
	{
		history.undo();
	}
	
	void HistorySystem::on_redo_command(const RedoCommand& redo_command)
	{
		history.redo();
	}
	
	void HistorySystem::on_store_snapshot_command(const StoreSnapshotCommand& snapshot_command)
	{
		history.store(HistoryEntry { snapshot_command.snapshot });
	}

	MetaAny HistorySystem::store_component(Registry& registry, Entity entity, const MetaType& component_type, bool allow_emplace, bool update_cursor)
	{
		if (!component_type)
		{
			return false;
		}

		auto history_comp = (allow_emplace)
			? get_or_emplace_history_component(registry, entity, component_type)
			: get_history_component_ref(registry, entity, component_type)
		;

		if (!history_comp)
		{
			if (allow_emplace)
			{
				print_warn("Failed to initialize history component for component type: {} (#{})", get_known_string_from_hash(component_type.id()), component_type.id());
			}

			return {};
		}

		bool result = (update_cursor)
			? store_component_history(registry, entity, history_comp)
			: component_history_store_back(registry, entity, history_comp)
		;

		if (!result)
		{
			print_warn("Failed to store historical data for component type: {} (#{})", get_known_string_from_hash(component_type.id()), component_type.id());

			return {};
		}

		return history_comp;
	}

	MetaAny HistorySystem::store_component(Registry& registry, Entity entity, MetaTypeID component_type_id, bool allow_emplace, bool update_cursor)
	{
		return store_component(registry, entity, resolve(component_type_id), allow_emplace, update_cursor);
	}

	MetaAny HistorySystem::store_back_component(Registry& registry, Entity entity, const MetaType& component_type)
	{
		return store_component(registry, entity, component_type, false, false);
	}

	MetaAny HistorySystem::store_back_component(Registry& registry, Entity entity, MetaTypeID component_type_id)
	{
		return store_component(registry, entity, component_type_id, false, false);
	}

	MetaAny HistorySystem::remove_latest_component_store(Registry& registry, Entity entity, const MetaType& component_type)
	{
		if (!component_type)
		{
			return {};
		}

		auto history_comp = get_history_component_ref(registry, entity, component_type);

		if (!history_comp)
		{
			return {};
		}

		if (!component_history_truncate_back(registry, entity, history_comp))
		{
			print_warn("Failed to truncate latest historical data for component type: {} (#{})", get_known_string_from_hash(component_type.id()), component_type.id());

			return {};
		}

		return history_comp;
	}

	MetaAny HistorySystem::remove_latest_component_store(Registry& registry, Entity entity, MetaTypeID component_type_id)
	{
		return remove_latest_component_store(registry, entity, resolve(component_type_id));
	}
}