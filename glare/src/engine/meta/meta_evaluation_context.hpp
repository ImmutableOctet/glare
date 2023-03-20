#pragma once

#include "types.hpp"

//#include <engine/types.hpp>

namespace engine
{
	class MetaVariableEvaluationContext;

	struct MetaEvaluationContext
	{
		MetaVariableEvaluationContext* variable_context = nullptr;
	};
}