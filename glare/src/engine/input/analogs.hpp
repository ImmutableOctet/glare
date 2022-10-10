#pragma once

#include <types.hpp>
#include <app/input/types.hpp>

#include <math/types.hpp>

#include <cstddef>

namespace engine
{
	using EngineAnalogsRaw = app::input::EngineAnalogsRaw;
	using AnalogsRaw = EngineAnalogsRaw;
	using EngineAnalogMap = app::input::EngineAnalogMap;

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

		DirectionVector get_analog(Analog analog) const;
		void set_analog(Analog analog, const DirectionVector& value);

		float angle_of(const DirectionVector& analog) const;
		float angle_of(Analog analog) const;

		float movement_angle() const;
		float camera_angle() const;
		float menu_select_angle() const;
		float orientation_angle() const;
	};

	// Analog input types. (represents an offset into `InputAnalogStates`)
	enum class Analog : AnalogsRaw
	{
		Movement    = offsetof(InputAnalogStates, movement),
		Camera      = offsetof(InputAnalogStates, camera),
		Zoom        = offsetof(InputAnalogStates, zoom),
		MenuSelect  = offsetof(InputAnalogStates, menu_select),
		Orientation = offsetof(InputAnalogStates, orientation),

		// NOTE: No other value, excluding `offsetof` expressions may be specified.
	};

	// TODO: Move this to a different header/file.
	// Maps analog names to their respective byte offset values in `InputAnalogStates`.
	// (Generated via `magic_enum`)
	void generate_analog_map(EngineAnalogMap& analogs);
}