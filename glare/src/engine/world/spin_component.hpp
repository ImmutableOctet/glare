#pragma once

#include <types.hpp>

#include <math/math.hpp>
#include <engine/types.hpp>

namespace engine
{
	struct Transform;

	struct SpinBehavior
	{
		static void update(World& world);

		math::Vector spin_vector = {0.0f, 0.05f, 0.0f};

		void apply(World& world, Entity entity, Transform& transform);
	};
}