#pragma once

//#include <engine/events.hpp>
#include <engine/types.hpp>

#include "contact_type.hpp"

namespace engine
{
	// General-purpose collision event type.
	struct OnCollision
	{
		// Contactor entity.
		Entity a;

		// Contacted entity.
		Entity b;

		// World-space position where the collision took place.
		math::Vector position;

		// The normal vector of the contacted geometry.
		// (Usually directed away from the point of contact)
		math::Vector normal;

		// The penetration depth of `a` into `b`, if applicable.
		float penetration_depth;

		// The type of contact that triggered this event.
		ContactType contact_type;
	};

	// TODO: Look into splitting the different `ContactType` enum values into their own types.
}