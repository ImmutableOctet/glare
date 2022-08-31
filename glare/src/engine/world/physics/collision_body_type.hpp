#pragma once

#include <types.hpp> // Global header.

namespace engine
{
	// TODO: Determine if this needs to be handled as a bitfield.
	enum class CollisionBodyType : std::uint8_t // unsigned char
	{
		Static    = (0),
		
		Basic     = (1),
		Kinematic = (1 << 1),
		Ghost     = (1 << 2),

		Dynamic = Kinematic,
	};

	FLAG_ENUM(std::uint8_t, CollisionBodyType); // unsigned char
}