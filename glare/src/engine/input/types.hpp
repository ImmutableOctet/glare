#pragma once

#include <engine/types.hpp>

#include <variant>
#include <functional>

namespace app::input
{
	class Mouse;
	class Keyboard;
	class Gamepad;
}

namespace engine
{
	// Placeholder type; empty for now.
	struct NetworkInputSource {};

	// Input source type; represents where an input came from.
	using InputSource = std::variant
	<
		std::monostate, // Used for unknown sources, as well frame/update-based `OnInput` event.

		std::reference_wrapper<const app::input::Mouse>,    // Mouse input.
		std::reference_wrapper<const app::input::Keyboard>, // Keyboard input.
		std::reference_wrapper<const app::input::Gamepad>,  // Gamepad input.

		std::reference_wrapper<const NetworkInputSource>    // Network input.
	>;

	// Integer type used to indicate which input-state affects an object.
	using InputStateIndex = PlayerIndex; // std::uint16_t; // std::uint8_t;
}