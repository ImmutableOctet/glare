#pragma once

#include <engine/command.hpp>

#include <engine/meta/types.hpp>
#include <engine/meta/meta_evaluation_context_store.hpp>

namespace engine
{
	struct FunctionCommand : public Command
	{
		// Opaque (non-owning) reference to a `MetaFunctionCall`
		// instance or similar executable fragment.
		MetaAny function;

		// Optional evaluation context used when resolving `function`.
		MetaEvaluationContextStore context = {};
	};
}