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
		bool result = false;

		/*
		// Alternative implementation (Disabled due to possibility of dangling references):
		if (value.owner())
		{
			result = context.set(scope, name, MetaAny { value }, true, true); // std::move(value)
		}
		else
		{
			result = context.set(scope, name, value.as_ref(), true, true);
		}
		*/

		result = context.set(scope, name, MetaAny { value }, true, true); // std::move(value)

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