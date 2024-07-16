#pragma once

#include "reflect.hpp"

namespace engine
{
	// Reserved type, used primarily for reflection.
	struct Util {};

	template <> void reflect<Util>();
}