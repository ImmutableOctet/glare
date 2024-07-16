#pragma once

#include <engine/reflection.hpp>

namespace engine
{
	class MotionSystem;

	template <> void reflect<MotionSystem>();
}