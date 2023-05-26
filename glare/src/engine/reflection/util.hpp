#pragma once

#include "reflection.hpp"

namespace engine
{
	// Reserved type, used primarily for reflection.
	struct Util {};

	extern template void reflect<Util>();
}