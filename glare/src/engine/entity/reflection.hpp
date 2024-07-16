#pragma once

#include <engine/reflection.hpp>

namespace engine
{
	class EntitySystem;

	template <> void reflect<EntitySystem>();
}