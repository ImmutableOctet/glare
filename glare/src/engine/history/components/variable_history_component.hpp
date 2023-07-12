#pragma once

#include <engine/meta/types.hpp>

#include <util/small_vector.hpp>

namespace engine
{
	struct MetaVariableHistory
	{
		MetaSymbolID resolved_variable_name = {};
		std::vector<MetaAny> snapshots;
	};

	struct VariableHistoryComponent
	{
		util::small_vector<MetaVariableHistory, 8> variable_history;
	};
}