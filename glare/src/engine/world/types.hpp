#pragma once

#include <engine/types.hpp>

namespace engine
{
	enum class CameraProjection : std::uint8_t
	{
		Perspective,
		Orthographic,

		Default = Perspective
	};
}