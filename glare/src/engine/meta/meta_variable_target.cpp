#include "meta_variable_target.hpp"

#include "meta_evaluation_context.hpp"
#include "meta_variable_evaluation_context.hpp"

#include <utility>

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
		return set_impl(value, context);
	}
	
	MetaVariableTarget& MetaVariableTarget::set(MetaAny& value, const MetaEvaluationContext& context)
	{
		if (!context.variable_context)
		{
			return *this;
		}

		return set_impl(value, *context.variable_context, context);
	}

	MetaVariableTarget& MetaVariableTarget::set(MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		if (!context.variable_context)
		{
			return *this;
		}

		return set_impl(value, *context.variable_context, registry, entity, context);
	}

	template <typename ...Args>
	MetaVariableTarget& MetaVariableTarget::set_impl(MetaAny& value, MetaVariableEvaluationContext& variable_context, Args&&... args)
	{
		bool result = false;

		/*
		// Alternative implementation (Disabled due to possibility of dangling references):
		if (value.owner())
		{
			result = variable_context.set(scope, name, MetaAny { value }, true, true, std::forward<Args>(args)...); // std::move(value)
		}
		else
		{
			result = variable_context.set(scope, name, value.as_ref(), true, true, std::forward<Args>(args)...);
		}
		*/

		result = variable_context.set
		(
			scope, name,
			MetaAny { value }, // std::move(value)
			true, true,
			std::forward<Args>(args)...
		);

		assert(result);

		return *this;
	}
}