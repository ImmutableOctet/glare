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

	// Attempts to retrieve an instance of `component_type` attached to `entity`.
	// 
	// If an instance could not be resolved, this will try to emplace
	// a default-constructed instance, then return a reference to that object.
	// 
	// If default construction fails, this will return an empty `MetaAny` object.
	MetaAny get_or_emplace_component(Registry& registry, Entity entity, const MetaType& component_type);

	// Attempts to retrieve an instance of the type identified by `component_type_id` attached to `entity`.
	// 
	// If an instance could not be resolved, this will try to emplace
	// a default-constructed instance, then return a reference to that object.
	// 
	// If default construction fails, this will return an empty `MetaAny` object.
	MetaAny get_or_emplace_component(Registry& registry, Entity entity, MetaTypeID component_type_id);

	// Attempts to emplace a default-constructed instance of `component_type` to `entity`.
	MetaAny emplace_default_component(Registry& registry, Entity entity, const MetaType& component_type);

	// Attempts to emplace a default-constructed instance of the type identified by `component_type_id` to `entity`.
	MetaAny emplace_default_component(Registry& registry, Entity entity, MetaTypeID component_type_id);

	// Constructs and attaches a component instance for `entity` based on the description specified.
	MetaAny emplace_component
	(
		Registry& registry, Entity entity,
		const MetaTypeDescriptor& component,
		const MetaEvaluationContext* opt_evaluation_context=nullptr
	);

	// Attempts to update a component attached to `entity` using the description specified.
	// 
	// This returns false if the component does not currently exist for `entity`.
	// If `value_assignment` is false, no value-assignment actions will be performed. (Status only)
	bool update_component_fields
	(
		Registry& registry, Entity entity,
		const MetaTypeDescriptor& component,

		bool value_assignment=true, bool direct_modify=false,
		const MetaEvaluationContext* opt_evaluation_context=nullptr
	);

	// Attempts to mark a component of type `component_type` as patched (i.e. updated).
	// The return value of this function indicates if the operation was performed successfully.
	bool mark_component_as_patched(Registry& registry, Entity entity, const MetaType& component_type);

	// Attempts to mark a component of the type referenced by `component_type_id` as patched (i.e. updated).
	// The return value of this function indicates if the operation was performed successfully.
	bool mark_component_as_patched(Registry& registry, Entity entity, MetaTypeID component_type_id);

	// Retrieves a `MetaTypeID` referencing a specialization of `HistoryComponent`, based on `component_type`.
	MetaTypeID get_history_component_type_id(const MetaType& component_type);

	// Retrieves a `MetaTypeID` referencing a specialization of `HistoryComponent`,
	// based on the type identified by `component_type_id`.
	MetaTypeID get_history_component_type_id(MetaTypeID component_type_id);

	// Retrieves a `MetaType` for a specialization of `HistoryComponent`, based on `component_type`.
	MetaType get_history_component_type(const MetaType& component_type);

	// Retrieves a `MetaType` for a specialization of `HistoryComponent`,
	// based on the type identified by `component_type_id`.
	MetaType get_history_component_type(MetaTypeID component_type_id);

	// Attempts to retrieve a `HistoryComponent` instance based on `component_type`, attached to `entity`.
	// 
	// If an instance could not be resolved, this will try to emplace
	// a default-constructed instance, then return a reference to that object.
	// 
	// If default construction fails, this will return an empty `MetaAny` object.
	MetaAny get_or_emplace_history_component(Registry& registry, Entity entity, const MetaType& component_type);

	// Attempts to retrieve a `HistoryComponent` instance based on the type
	// identified by `component_type_id`, attached to `entity`.
	// 
	// If an instance could not be resolved, this will try to emplace
	// a default-constructed instance, then return a reference to that object.
	// 
	// If default construction fails, this will return an empty `MetaAny` object.
	MetaAny get_or_emplace_history_component(Registry& registry, Entity entity, MetaTypeID component_type_id);

	// Attempts to retrieve a `HistoryComponent` instance based on `component_type`, attached to `entity`.
	MetaAny get_history_component_ref(Registry& registry, Entity entity, const MetaType& component_type);

	// Attempts to retrieve a `HistoryComponent` instance based on the type
	// identified by `component_type_id`, attached to `entity`.
	MetaAny get_history_component_ref(Registry& registry, Entity entity, MetaTypeID component_type_id);

	// Wrapper function for invocation of the `undo` command of a `HistoryComponent`.
	bool undo_component_change(Registry& registry, Entity entity, MetaAny& history_component);

	// Wrapper function for invocation of the `redo` command of a `HistoryComponent`.
	bool redo_component_change(Registry& registry, Entity entity, MetaAny& history_component);

	// Wrapper function for invocation of the `store` command of a `HistoryComponent`.
	bool store_component_history(Registry& registry, Entity entity, MetaAny& history_component);

	// Wrapper function for invocation of the `truncate` command of a `HistoryComponent`.
	bool truncate_component_history(Registry& registry, Entity entity, MetaAny& history_component);

	// Wrapper function for invocation of the `store_back` command of a `HistoryComponent`.
	bool component_history_store_back(Registry& registry, Entity entity, MetaAny& history_component);

	// Wrapper function for invocation of the `truncate_back` command of a `HistoryComponent`.
	bool component_history_truncate_back(Registry& registry, Entity entity, MetaAny& history_component, bool update_cursor);

	// Wrapper function for invocation of the `truncate_back` command of a `HistoryComponent`.
	bool component_history_truncate_back(Registry& registry, Entity entity, MetaAny& history_component);
}