#pragma once

#include "reflect.hpp"

namespace engine
{
	// Reserved type, used primarily for reflection.
	struct Util {};

	extern template void reflect<Util>();
}