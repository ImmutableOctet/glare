#pragma once

#include "meta_variable_target.hpp"
#include "meta_variable_evaluation_context.hpp"

#include <engine/entity/entity_target.hpp>
#include <engine/entity/entity_thread_target.hpp>

namespace engine
{
	struct IndirectMetaVariableTarget
	{
		EntityTarget target;
		MetaVariableTarget variable;

		EntityThreadTarget thread = {};

		MetaAny get(const MetaVariableEvaluationContext& context) const;
		MetaAny get(const MetaEvaluationContext& context) const;
		MetaAny get(Registry& registry, Entity source) const;

		IndirectMetaVariableTarget& set(MetaAny& value, MetaVariableEvaluationContext& context);
		IndirectMetaVariableTarget& set(MetaAny& value, const MetaEvaluationContext& context);
		IndirectMetaVariableTarget& set(MetaAny& value, Registry& registry, Entity source);

		MetaVariableEvaluationContext get_variable_context(Registry& registry, Entity entity) const;

		bool operator==(const IndirectMetaVariableTarget&) const noexcept = default;
		bool operator!=(const IndirectMetaVariableTarget&) const noexcept = default;

		inline bool operator==(const MetaVariableTarget& value) const noexcept
		{
			return ((target.is_self_targeted()) && (variable == value));
		}

		inline bool operator!=(const MetaVariableTarget& value) const noexcept
		{
			return !operator==(value);
		}
	};
}