#pragma once

#include "types.hpp"

namespace engine
{
	struct MetaTypeDescriptor;
	struct MetaEvaluationContext;

    // Retrieves a reference to an instance of `component_type` attached to `entity`.
    // If an instance could not be resolved, an empty `MetaAny` will be returned.
    MetaAny get_component_ref(Registry& registry, Entity entity, const MetaType& component_type);

    // Retrieves a reference to an instance of the type identified by `component_type_id` attached to `entity`.
    // If an instance could not be resolved, or if a type could not be resolve from `component_type_id`, an empty `MetaAny` will be returned.
    MetaAny get_component_ref(Registry& registry, Entity entity, MetaTypeID component_type_id);

	MetaAny emplace_component
	(
		Registry& registry, Entity entity,
		const MetaTypeDescriptor& component,
		const MetaEvaluationContext* opt_evaluation_context=nullptr
	);

	// Returns false if the component does not currently exist for `entity`.
	// If `value_assignment` is false, no value-assignment actions will be performed. (Status only)
	bool update_component_fields
	(
		Registry& registry, Entity entity,
		const MetaTypeDescriptor& component,

		bool value_assignment=true, bool direct_modify=false,
		const MetaEvaluationContext* opt_evaluation_context=nullptr
	);
}