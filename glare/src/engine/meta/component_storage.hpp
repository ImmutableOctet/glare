#pragma once

#include "types.hpp"
//#include "meta/types.hpp"

namespace engine
{
	// Abstraction around type-erased storage of multiple entity components.
	struct ComponentStorage
	{
		// Collection of component instances to be
		// re-attached to the underlying entity.
		MetaStorage components;

		std::size_t store(Registry& registry, Entity entity, const MetaStorageDescription& component_details, bool store_as_copy=false, bool skip_existing=false);
		std::size_t retrieve(Registry& registry, Entity entity);

		bool empty() const;

		inline explicit operator bool() const { return !empty(); }

		void clear();
	};
}