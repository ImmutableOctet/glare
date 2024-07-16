#pragma once

#include <engine/reflection.hpp>

namespace engine
{
	class AnimationSystem;

	template <> void reflect<AnimationSystem>();
}