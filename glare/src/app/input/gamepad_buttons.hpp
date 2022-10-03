#pragma once

#include "types.hpp"

namespace app::input
{
	// Enumeration containing button names and their corresponding bit position.
	// This is a type-safe version of `GamepadButtonID` since it's statically
	// known what buttons a gamepad is supposed to have. (Assumes XB/PS layout via SDL)
	// NOTE: This enumeration type's values need to remain in sync with `GamepadState::Buttons`.
	enum class GamepadButtonBits : GamepadButtonID
	{
		// Face buttons:
		A, // (1 << 0)
		B, // (1 << 1)
		X, // (1 << ...)
		Y, // ...

		// Bumpers:
		LB,
		RB,

		// Middle face buttons:
		Select, // Left-middle
		Start, // Right-middle

		// Analog stick buttons:
		L3,
		R3,

		// D-Pad:
		UP,
		RIGHT,
		DOWN,
		LEFT
	};
}