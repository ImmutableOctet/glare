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

		MetaAny resolve_singleton_from_type(const MetaType& type) const;
		MetaAny resolve_singleton_from_type_id(const MetaTypeID type_id) const;
	};
}