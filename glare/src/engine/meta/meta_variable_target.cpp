#include "meta_variable_target.hpp"

#include "meta_evaluation_context.hpp"
#include "meta_variable_evaluation_context.hpp"

namespace engine
{
	MetaAny MetaVariableTarget::get(const MetaVariableEvaluationContext& context) const
	{
		return context.get(scope, name);
	}

	MetaAny MetaVariableTarget::get(const MetaEvaluationContext& context) const
	{
		if (!context.variable_context)
		{
			return {};
		}

		return get(*context.variable_context);
	}

	MetaAny MetaVariableTarget::get(Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return get(context);
	}

	MetaVariableTarget& MetaVariableTarget::set(MetaAny& value, MetaVariableEvaluationContext& context)
	{
		const auto result = context.set(scope, name, std::move(value), true, true);

		assert(result);

		return *this;
	}
	
	MetaVariableTarget& MetaVariableTarget::set(MetaAny& value, const MetaEvaluationContext& context)
	{
		if (!context.variable_context)
		{
			return *this;
		}

		return set(value, *context.variable_context);
	}

	MetaVariableTarget& MetaVariableTarget::set(MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		return set(value, context);
	}
}