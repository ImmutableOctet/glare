#pragma once

#include <engine/history/history_entry.hpp>

#include <engine/command.hpp>

namespace engine
{
	struct StoreSnapshotCommand : public Command
	{
		HistoryEntry snapshot;
	};
}