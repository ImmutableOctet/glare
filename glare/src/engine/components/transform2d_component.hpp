#pragma once

#include <math/types.hpp>

namespace engine
{
	struct Transform2DComponent
	{
		math::Vector2D position;

		float rotation;

		//math::Vector2D scale = { 1.0f, 1.0f };
	};
}