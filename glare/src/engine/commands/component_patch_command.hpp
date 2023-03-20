#pragma once

#include <engine/types.hpp>
#include <engine/command.hpp>
//#include <engine/meta/types.hpp>
//#include <engine/meta/meta_type_descriptor.hpp>

#include <engine/meta/meta_evaluation_context_store.hpp>

namespace engine
{
	struct MetaTypeDescriptor;
	struct MetaEvaluationContext;

	// Updates the component-type described by `component` for `target`.
	struct ComponentPatchCommand : public Command
	{
		// NOTE: Use of raw pointer types.
		using ComponentChanges = const MetaTypeDescriptor*;
		using EvaluationContext = const MetaEvaluationContext*;

		ComponentChanges component = {};
		
		// Optional evaluation context used when resolving `component`.
		MetaEvaluationContextStore context = {};
	};
}