#pragma once

#include "types.hpp"

namespace engine
{
	// This acts as a simple abstraction around type-erased access to a data-member of a type.
	struct MetaDataMember
	{
		// The identifier for the type this data member belongs to.
		MetaTypeID type_id;

		// A type-local data identifier used to retrieve a value from an object instance of `type_id`.
		MetaSymbolID data_member_id;

		// Retrieves the data-member referenced by `data_member_id` from `instance`.
		// If `instance` does not have the same identifier as `type_id`, this will return an empty object.
		MetaAny get(const MetaAny& instance) const;

		// Retrieves the data-member referenced by `data_member_id`
		// from an instance of `type_id` attached to `entity` in `registry`.
		// 
		// If an instance of `type_id` is not currently attached to `entity`, this will return an empty object.
		MetaAny get(Registry& registry, Entity entity) const;
	};
}