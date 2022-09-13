#pragma once

#include <types.hpp>

namespace engine
{
	enum class ContactType : std::uint8_t
	{
		// Reserved; not currently in use.
		None = 0,

		// For event-triggers, zones, etc.
		//EventTrigger,

		// Indicates that an object is touching another object, but not intersecting it.
		// This is commonly produced from a convex-cast or ray-cast.
		Surface,

		// Indicates that one object is overlapping another.
		// This is usually the result of an intersection-resolution.
		Intersection,

		// For scenarios where an object passes entirely inside of another object.
		Enclosure,

		// For scenarios where an object that had previously been inside of an another object escapes.
		Exclosure,
	};
}