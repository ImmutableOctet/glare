#pragma once

#include "types.hpp"

//#include <engine/types.hpp>

namespace engine
{
	class MetaVariableEvaluationContext;
	class Service;
	class SystemManagerInterface;

	struct MetaEvaluationContext
	{
		MetaVariableEvaluationContext* variable_context = nullptr;
		Service* service = nullptr;
		SystemManagerInterface* system_manager = nullptr;
	};
}