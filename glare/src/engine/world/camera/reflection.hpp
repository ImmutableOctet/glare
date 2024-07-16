#pragma once

#include <engine/reflection.hpp>

namespace engine
{
	class CameraSystem;

	template <> void reflect<CameraSystem>();
}