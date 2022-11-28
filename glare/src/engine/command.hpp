#pragma once

#include "types.hpp"

namespace engine
{
	struct Command
	{
		Entity source = null;
		Entity target = null;

		inline Entity entity() const
		{
			return target;
		}
	};
}