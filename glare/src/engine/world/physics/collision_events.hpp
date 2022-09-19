#pragma once

#include "contact_type.hpp"

//#include <engine/events.hpp>
#include <engine/types.hpp>
#include <math/types.hpp>

class btCollisionObject;

namespace engine
{
	// NOTE: AABB overlaps are triggered on both 'collidable' and 'interactable' objects.
	struct OnAABBOverlap // CollisionIntersectionMetadata
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
		float penetration() const;

		// The speed of the contact made to entity `b`'s surface.
		// (Movement speed of `a`; length of `impact_velocity`)
		float contact_speed() const;

		// Entity `a`'s angle of approach.
		// (Normalized direction of `impact_velocity`)
		math::Vector direction() const;

		// Length-vector indicating the intended penetration of this surface.
		// (`a`'s intended movement-depth into `b` along the `direction` of approach)
		math::Vector penetration_vector() const;

		// Force applied to the surface.
		// (Difference between contact-speed and penetration amount)
		float force_applied() const;

		// Ratio of `force_applied` to the `penetration` observed.
		float residual() const;

		// The slope of the contacted surface; computed from the
		// surface `normal` and the movement `direction` imposed by `a`.
		// Ranges from -1.0 (downward) to 1.0 (upward) along the surface.
		float slope() const;

		// The slope of the contacted surface.
		// (Dot-product of the `forward` vector and the `direction`)
		float slope(const math::Vector& surface_forward) const;

		// The slope of the contacted surface.
		// (Calls the surface-forward overload of `slope` with the second (`forward`) vector)
		float slope(const math::OrthogonalVectors& orientation) const;

		// Retrieves the three orthogonal vectors representing the orientation of the surface.
		math::OrthogonalVectors surface_orientation_vectors(const math::Vector& adjacent={1.0f, 0.0f, 0.0f}) const;
	};

	// This is triggered any time a standard collision intersection takes place.
	// (For resolvable collisions)
	struct OnIntersection
	{
		// General collision information.
		OnCollision collision;

		// The positional-correction applied to `a`.
		math::Vector correction;
	};

	// This is triggered similarly to `OnIntersection` but in the case of interactable objects.
	// This event type and `OnIntersection` are not mutually exclusive, and could trigger for the same intersection.
	// By convention, this is unlikely, but possible. For this reason, the internal `collision` object is not event-aliased.
	struct OnInteractionIntersection
	{
		OnCollision collision;

		// The entity that instigated the interaction.
		inline Entity instigator() const
		{
			return collision.a;
		}

		// The entity being interacted with.
		inline Entity interacted_with() const
		{
			return collision.b;
		}
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