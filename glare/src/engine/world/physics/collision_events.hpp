#pragma once

#include "contact_type.hpp"

//#include <engine/events.hpp>
#include <engine/types.hpp>
#include <math/types.hpp>

class btCollisionObject;

namespace engine
{
	struct OnAABBOverlap
	{
		// The target entity that is within the AABB of `bounding`.
		Entity entity;

		// The entity whose AABB is intersecting `entity`.
		Entity bounding;

		// The number of contacts between `entity` and `bounding`.
		int number_of_contacts;

		struct
		{
			const btCollisionObject* entity_collision_obj;
			const btCollisionObject* bounding_collision_obj;
		} native;
	};

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

		// Penetration amount into `b` from `a`.
		float penetration;

		// The type of contact that triggered this event.
		ContactType contact_type;
	};

	struct OnSurfaceContact
	{
		// General collision information.
		// (Forwarded as `OnCollision` event via `PhysicsSystem::on_surface_contact`)
		OnCollision collision;

		/*
			A length-vector indicating the original move of
			entity `a` into/toward entity `b`.
			
			See also `approach_angle` method.
		*/
		math::Vector impact_velocity;

		// Shorthand for a field of the same name, found in `collision`.
		inline float penetration() const
		{
			return collision.penetration;
		}

		// The speed of the contact made to entity `b`'s surface.
		// (Movement speed of `a`; length of `impact_velocity`)
		inline float contact_speed() const
		{
			return glm::length(impact_velocity);
		}

		// Entity `a`'s angle of approach.
		// (Normalized direction of `impact_velocity`)
		inline math::Vector direction() const
		{
			return glm::normalize(impact_velocity);
		}

		// Length-vector indicating the intended penetration of this surface.
		// (`a`'s intended movement-depth into `b` along the `direction` of approach)
		inline math::Vector penetration_vector() const
		{
			return (direction() * penetration());
		}

		inline float force_applied() const
		{
			return (contact_speed() - penetration());
		}

		// Ratio of `force_applied` to the `penetration` observed.
		inline float residual() const
		{
			return (force_applied() / penetration());
		}

		// The slope of the contacted surface; computed from the
		// surface `normal` and the movement `direction` imposed by `a`.
		// Ranges from -1.0 (downward) to 1.0 (upward) along the surface.
		inline float slope() const
		{
			return math::get_surface_slope(collision.normal, direction());
		}
	};

	struct OnIntersection
	{
		// General collision information.
		OnCollision collision;

		// The positional-correction applied to `a`.
		math::Vector correction;
	};

	// TODO: Look into splitting the different `ContactType` enum values into their own types.

	// Triggered when one kinematic object influences the position of another kinematic object. (e.g. object A pushes object B)
	// These events are normally the result of a kinematic body's position changing. (With a kinematic resolution-method configured)
	struct OnKinematicInfluence
	{
		Entity influencer;

		struct
		{
			Entity entity;

			math::Vector old_position;
			math::Vector new_position;

			math::Vector influence_applied;
		} target;

		struct
		{
			math::Vector point;
			math::Vector normal;
		} contact;
	};

	/*
		This event is triggered any time a kinematic object's path is affected by another object.
		(e.g. A player tries to move forward but is blocked by a wall)
		
		This event triggers if any adjustment is made, including small adjustments from influencing other objects.
		
		This event does NOT trigger if the kinematic object's exact destination is reached by influencing another object.
		(i.e. an object in the path of the entity is moved out of the way, allowing the entity to move unaffected)
		
		See also: `OnKinematicInfluence`
	*/
	struct OnKinematicAdjustment
	{
		// The entity being adjusted.
		Entity entity;

		// The cause of the adjustment.
		// (e.g. object in the path of `entity`)
		Entity adjusted_by;

		// The previously established position of `entity`.
		math::Vector old_position;

		// The newly adjusted position of `entity`.
		math::Vector adjusted_position;

		// The 'expected' position of `entity`, before interacting with `adjusted_by`.
		math::Vector expected_position;
	};
}