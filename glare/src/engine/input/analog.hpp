#pragma once

#include "types.hpp"

#include <app/input/types.hpp>

#include <math/types.hpp>

#include <cstdint>
#include <cstddef>

namespace engine
{
	using EngineAnalogsRaw = app::input::EngineAnalogsRaw;
	using AnalogsRaw       = EngineAnalogsRaw;

	using EngineAnalogMap  = app::input::EngineAnalogMap;

	class Service;

	struct InputState;
}

namespace game
{
	using EngineAnalogsRaw = engine::EngineAnalogsRaw;
	using AnalogsRaw       = engine::AnalogsRaw;
	using EngineAnalogMap  = engine::EngineAnalogMap;

	enum class Analog : AnalogsRaw;

	struct InputAnalogStates
	{
		using DirectionVector = math::Vector2D;

		                             // Gamepad equivalent:
		DirectionVector movement;    // Left stick
		DirectionVector camera;      // Right stick
		
		// Camera zoom; X axis is reserved, only Y axis is used under normal conditions.
		DirectionVector zoom;        // No gamepad equivalent (yet)

		DirectionVector menu_select; // D-Pad

		// TODO: Ability-related - need to finalize control-scheme.
		DirectionVector orientation; // Triggers...?

		// NOTE: This function is implemented at game-level.
		DirectionVector get_analog(Analog analog) const;

		// NOTE: This function is implemented at game-level.
		void set_analog(Analog analog, const DirectionVector& value);

		// NOTE: This function is implemented at game-level.
		void emit_continuous_input_events(engine::Service& service, engine::InputStateIndex input_state_index, const engine::InputState& input_state) const;

		// Returns true if the length of the direction vector for `analog` is greater than the threshold specified.
		bool has_analog_input(Analog analog, float minimum_threshold=0.0f) const;

		float angle_of(const DirectionVector& analog) const;
		float angle_of(Analog analog) const;

		float movement_angle() const;
		float camera_angle() const;
		float menu_select_angle() const;
		float orientation_angle() const;
	};

	// NOTE: This must be defined at game-level.
	// Maps analog names to their respective byte offset values in `InputAnalogStates`.
	// (Generated via `magic_enum`)
	void generate_analog_map(EngineAnalogMap& analogs);
}

namespace engine
{
	using Analog            = game::Analog;
	using InputAnalogStates = game::InputAnalogStates;
}