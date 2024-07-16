#pragma once

#include <engine/reflection.hpp>

namespace engine
{
	class DebugListener;

	template <> void reflect<DebugListener>();
}