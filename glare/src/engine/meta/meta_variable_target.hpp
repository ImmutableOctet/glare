#pragma once

#include "types.hpp"
#include "meta_variable_scope.hpp"

namespace engine
{
	struct MetaEvaluationContext;
	class MetaVariableEvaluationContext;

	struct MetaVariableTarget
	{
		// The (resolved) name of the targeted variable.
		MetaSymbolID name = {};

		// The scope of the targeted variable.
		MetaVariableScope scope = MetaVariableScope::Local;

		MetaAny get(const MetaVariableEvaluationContext& context) const;
		MetaAny get(const MetaEvaluationContext& context) const;
		MetaAny get(Registry& registry, Entity entity, const MetaEvaluationContext& context) const;

		MetaVariableTarget& set(MetaAny& value, MetaVariableEvaluationContext& context);
		MetaVariableTarget& set(MetaAny& value, const MetaEvaluationContext& context);
		MetaVariableTarget& set(MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context);

		//MetaVariableTarget& operator=(const MetaVariableTarget&) = default;
		//MetaVariableTarget& operator=(MetaVariableTarget&&) noexcept = default;

		bool operator==(const MetaVariableTarget&) const noexcept = default;
		bool operator!=(const MetaVariableTarget&) const noexcept = default;
	};
}