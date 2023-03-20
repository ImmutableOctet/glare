#include "meta_evaluation_context_store.hpp"

#include <utility>

namespace engine
{
	MetaEvaluationContextStore::MetaEvaluationContextStore(MetaVariableEvaluationContext&& variable_context)
		: variable_context(std::move(variable_context))
	{}

	MetaEvaluationContextStore::MetaEvaluationContextStore(const MetaEvaluationContext& context)
		: variable_context(*context.variable_context) {}

	MetaEvaluationContext MetaEvaluationContextStore::get_context() const
	{
		return MetaEvaluationContext
		{
			&variable_context
		};
	}

	const MetaVariableEvaluationContext& MetaEvaluationContextStore::get_variable_context() const
	{
		return variable_context;
	}
}