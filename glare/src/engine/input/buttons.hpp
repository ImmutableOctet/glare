#pragma once

#include <app/input/types.hpp>

namespace engine
{
	// Raw integral type used as a bitfield for button states.
	using EngineButtonsRaw = app::input::EngineButtonsRaw; // std::uint32_t; // std::uint64_t;
	using ButtonsRaw       = EngineButtonsRaw;

	// Equivalent to `std::unordered_map<std::string, EngineButtonsRaw>`.
	using EngineButtonMap  = app::input::EngineButtonMap;
}

namespace game
{
	using EngineButtonsRaw = engine::EngineButtonsRaw;
	using ButtonsRaw       = engine::ButtonsRaw;
	using EngineButtonMap  = engine::EngineButtonMap;

	enum class Button : ButtonsRaw;

	// NOTE: This must be defined at game-level.
	// Maps button names to their bitwise positional values/masks.
	// (Generated via `magic_enum`)
	void generate_button_map(EngineButtonMap& buttons);
}

namespace engine
{
	using Button = game::Button;
}