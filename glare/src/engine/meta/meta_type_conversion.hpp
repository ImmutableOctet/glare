#pragma once

#include "types.hpp"

namespace engine
{
	struct MetaEvaluationContext;

	// Converts an object to the type identified by `type_id`.
	struct MetaTypeConversion
	{
		// An identifier representing the type to cast to.
		MetaTypeID type_id = {};

		MetaTypeID get_type_id() const;
		MetaType get_type() const;

		// Attempts to default-construct the type referenced by `type_id`.
		MetaAny get() const;

		// Attempts to default-construct the type referenced by `type_id`.
		// NOTE: This overload has been added for maximum compatibility.
		MetaAny get(Registry& registry, Entity entity) const;

		// Attempts to default-construct the type referenced by `type_id`.
		// NOTE: This overload has been added for maximum compatibility.
		MetaAny get(Registry& registry, Entity entity, const MetaEvaluationContext& context) const;

		// Attempts to cast or construct the type referenced by `type_id`, using `instance`.
		// If `instance` is empty, this will return an empty object as well.
		// If `instance` already has the type referenced by `type_id`, this will return a reference to `instance`.
		MetaAny get(const MetaAny& instance) const;

		// Attempts to cast or construct the type referenced by `type_id`, using `instance`.
		// NOTE: This overload has been added for maximum compatibility.
		MetaAny get(const MetaAny& instance, Registry& registry, Entity entity) const;

		// Attempts to cast or construct the type referenced by `type_id`, using `instance`.
		// NOTE: This overload has been added for maximum compatibility.
		MetaAny get(const MetaAny& instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const;
	};
}