#pragma once

#include "types.hpp"

#include <util/enum_operators.hpp>
#include <math/math.hpp>

namespace app::input
{
	// Per-device virtual button definition.
	// Converts from a device-native analog input to an engine-defined button input.
	struct VirtualButton
	{
		enum class ComparisonMethod : std::uint8_t
		{
			// Multidirectional.
			Both,

			// Unidirectional:
			Greater,
			Lesser,
		};

		using AxisRaw = std::uint8_t;

		enum class Axis : AxisRaw
		{
			X = (1 << 0),
			Y = (1 << 1),
			Z = (1 << 2),

			XYZ = (X|Y|Z),
			XY  = (X|Y),
			XZ  = (X|Z),
			YZ  = (Y|Z),
		};

		bool is_down(const math::Vector2D& values) const;
		bool is_down(const math::Vector3D& values) const;
		bool is_down(float value) const;

		EngineButtonsRaw engine_button;
		float threshold;

		Axis analog_axis;
		ComparisonMethod comparison;
	};

	FLAG_ENUM(VirtualButton::AxisRaw, VirtualButton::Axis);
}