#pragma once

namespace engine
{
	struct ServicePolicy
	{
		bool destroy_children_with_parent : 1 = true;
	};
}