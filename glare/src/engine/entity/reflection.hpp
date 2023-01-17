#pragma once

#include <engine/reflection.hpp>

namespace engine
{
	class EntitySystem;

	extern template void reflect<EntitySystem>();
}