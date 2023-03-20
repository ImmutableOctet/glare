#pragma once

#include "meta_evaluation_context.hpp"
#include "meta_variable_evaluation_context.hpp"

namespace engine
{
	// Temporary storage for a `MetaEvaluationContext` and its contents.
	// 
	// TODO: Implement functionality that keeps track of the internal
	// shared pointers used to establish `MetaVariableEvaluationContext`.
	struct MetaEvaluationContextStore
	{
		public:
			MetaEvaluationContextStore() = default;
			MetaEvaluationContextStore(MetaVariableEvaluationContext&& variable_context);
			MetaEvaluationContextStore(const MetaEvaluationContext& context);

			// Creates a temporary `MetaEvaluationContext` handle to this storage object.
			MetaEvaluationContext get_context() const;

			const MetaVariableEvaluationContext& get_variable_context() const;
		protected:
			mutable MetaVariableEvaluationContext variable_context;
	};
}