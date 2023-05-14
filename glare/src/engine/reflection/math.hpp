#pragma once

#include "reflection.hpp"

namespace engine
{
	// Reserved type, used primarily for reflection.
	struct Math {};

	extern template void reflect<Math>();
}