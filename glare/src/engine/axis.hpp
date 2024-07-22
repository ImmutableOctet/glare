#pragma once

namespace engine
{
	// TODO: Rename to something less generic. (i.e. RotationAxis)
	enum class Axis : std::uint8_t
	{
		Pitch = (1 << 0),
		Yaw   = (1 << 1),
		Roll  = (1 << 2),
	};
	
	FLAG_ENUM(std::uint8_t, Axis);
}