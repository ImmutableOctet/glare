#pragma once

#include "types.hpp"

namespace engine
{
	struct OnStateChange
	{
		Entity entity;

		EntityStateInfo from;
		EntityStateInfo to;
	};
}