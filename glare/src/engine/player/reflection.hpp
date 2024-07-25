#pragma once

#include <engine/reflection.hpp>

namespace engine
{
	class PlayerSystem;

	template <> void reflect<PlayerSystem>();
}