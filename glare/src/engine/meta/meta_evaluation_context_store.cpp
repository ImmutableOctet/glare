#include "meta_evaluation_context_store.hpp"

#include <utility>

namespace engine
{
	MetaEvaluationContextStore::MetaEvaluationContextStore
	(
		MetaVariableEvaluationContext&& variable_context,
		Service* service,
		SystemManagerInterface* system_manager
	) :
		variable_context(std::move(variable_context)),
		service(service),
		system_manager(system_manager)
	{}

	MetaEvaluationContextStore::MetaEvaluationContextStore(const MetaEvaluationContext& context) :
		variable_context(*context.variable_context),
		service(context.service),
		system_manager(context.system_manager)
	{}

	MetaEvaluationContext MetaEvaluationContextStore::get_context() const
	{
		return MetaEvaluationContext
		{
			&variable_context,
			service,
			system_manager
		};
	}

	const MetaVariableEvaluationContext& MetaEvaluationContextStore::get_variable_context() const
	{
		return variable_context;
	}

	const Service* MetaEvaluationContextStore::get_service() const // Service*
	{
		return service;
	}

	const SystemManagerInterface* MetaEvaluationContextStore::get_system_manager() const // SystemManagerInterface*
	{
		return system_manager;
	}
}