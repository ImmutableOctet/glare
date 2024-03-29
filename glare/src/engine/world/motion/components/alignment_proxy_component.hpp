#pragma once

#include <engine/types.hpp>

namespace engine
{
	// Similar to `ForwardingComponent`, this specifies a proxy entity for alignment to be applied.
	struct AlignmentProxyComponent
	{
		Entity entity = null;
	};
}