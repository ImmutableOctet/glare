#pragma once

#include <engine/reflection.hpp>

namespace engine
{
	class PhysicsSystem;

	template <> void reflect<PhysicsSystem>();
}