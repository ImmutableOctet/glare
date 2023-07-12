#pragma once

#include "history_entry.hpp"

#include <util/history_log.hpp>

namespace engine
{
	using HistoryLog = util::HistoryLog<HistoryEntry>;
}