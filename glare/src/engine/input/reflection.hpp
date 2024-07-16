#pragma once

#include <engine/reflection.hpp>

#include "analog.hpp"
#include "buttons.hpp"

namespace engine
{
	class InputSystem;

	template <> GLARE_GAME_API void reflect<InputSystem>();

	// NOTE: Defined at game-level.
	template <> GLARE_GAME_IMPORT void reflect<Button>();

	// NOTE: Defined at game-level.
	template <> GLARE_GAME_IMPORT void reflect<Analog>();
}