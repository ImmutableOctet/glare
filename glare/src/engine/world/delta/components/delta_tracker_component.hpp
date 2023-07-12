#pragma once

#include <engine/meta/meta_type_id_filter.hpp>

namespace engine
{
	// A filter to control which components are to be tracked for a given entity.
	struct DeltaTrackerComponent
	{
		MetaTypeIDFilter<> tracked_components;
	};
}