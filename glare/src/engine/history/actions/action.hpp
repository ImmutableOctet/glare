#pragma once

#include "component_creations.hpp"
#include "component_destructions.hpp"
#include "component_modifications.hpp"

#include <variant>

namespace engine
{
	namespace history
	{
		using Action = std::variant
		<
			ComponentCreations,
			ComponentDestructions,
			ComponentModifications
		>;
	}

	using HistoryAction = history::Action;
}