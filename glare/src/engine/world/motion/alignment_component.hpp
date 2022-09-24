#pragma once

#include <engine/types.hpp>

namespace engine
{
	// Similar to `ForwardingComponent`, this specifies a proxy entity for alignment to be applied.
	// If this component is attached to an entity with a `MotionComponent` whose ``
	struct AlignmentComponent
	{
		Entity entity = null;
	};
}