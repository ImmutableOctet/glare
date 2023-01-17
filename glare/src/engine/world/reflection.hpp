#pragma once

#include <engine/reflection.hpp>

#include "animation/reflection.hpp"
#include "behaviors/reflection.hpp"
#include "motion/reflection.hpp"
#include "physics/reflection.hpp"
// ...

namespace engine
{
	class World;

	extern template void reflect<World>();
}