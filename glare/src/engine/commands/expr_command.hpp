#pragma once

#include <engine/command.hpp>

#include <engine/meta/types.hpp>
#include <engine/meta/meta_evaluation_context_store.hpp>

namespace engine
{
	// See also: `FunctionCommand`
	struct ExprCommand : public Command
	{
		// Opaque (non-owning) reference to a `MetaValueOperation`
		// instance or similar executable fragment.
		MetaAny expr;

		// Optional evaluation context used when resolving `expr`.
		MetaEvaluationContextStore context = {};
	};
}