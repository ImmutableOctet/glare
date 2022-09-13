#pragma once

#include "collision_cast_result.hpp"
#include "collision_group.hpp"

#include <math/math.hpp>

#include <optional>
#include <variant>

#include <bullet/LinearMath/btVector3.h>
#include <bullet/LinearMath/btTransform.h>

//class btVector3;
//class btTransform;

class btConvexShape;

namespace engine
{
	class PhysicsSystem;
	struct CollisionComponent;

	// Destination for a cast operation; can either be a world-space position or a matrix.
	using CastDestination = std::variant<math::Matrix, math::Vector>;

	std::optional<ConvexCastResult> convex_cast_impl
	(
		PhysicsSystem& physics,
		
		const btCollisionObject& collision_obj_ref,
		const btConvexShape& shape,

		const btTransform& from_matrix_bt,
		const btTransform& to_matrix_bt,

		std::optional<btVector3> from_position_bt=std::nullopt,
		std::optional<btVector3> to_position_bt=std::nullopt,

		std::optional<int> filter_group=std::nullopt,         // Defaults to `collision_obj_ref`'s filter.
		std::optional<int> filter_mask=std::nullopt,          // Defaults to `collision_obj_ref`'s mask.

		std::optional<float> allowed_penetration=std::nullopt // Defaults to the collision-world's allowable penetration.
	);

	// TODO: Add version of `convex_cast` that allows the use of shapes directly.
	// (Would need to either require CollisionGroups outright, or have them defaulted to `CollisionGroup::All`)
	// 
	// Performs a cast using a convex object (`entity`) between two points in world-space.
	// If a hit is detected, this function will return a `ConvexCastResult` object, otherwise, the return value will be `std::nullopt`.
	std::optional<ConvexCastResult> convex_cast_to
	(
		PhysicsSystem& physics,

		Entity entity,
		const CollisionComponent& collision,

		const CastDestination& destination,

		std::optional<CollisionGroup> filter_group=std::nullopt, // Defaults to `collision` object's filter.
		std::optional<CollisionGroup> filter_mask=std::nullopt,  // Defaults to `collision` object's mask.
		std::optional<float> allowed_penetration=std::nullopt    // Defaults to internal collision-world's allowable penetration.
	);

	// See main overload for details. (Shorthand overload)
	std::optional<ConvexCastResult> convex_cast_to
	(
		PhysicsSystem& physics,

		Entity entity,

		const CastDestination& destination,

		std::optional<CollisionGroup> filter_group=std::nullopt, // Defaults to `collision` object's filter.
		std::optional<CollisionGroup> filter_mask=std::nullopt,  // Defaults to `collision` object's mask.
		std::optional<float> allowed_penetration=std::nullopt    // Defaults to internal collision-world's allowable penetration.
	);

	/*
	std::optional<RayCastResult> ray_cast
	(
		Entity entity,
		const CollisionComponent& collision,

		const math::Vector& from,
		const math::Vector& to_direction,

		std::optional<CollisionGroup> filter_group=std::nullopt, // Defaults to `collision` object's filter.
		std::optional<CollisionGroup> filter_mask=std::nullopt,  // Defaults to `collision` object's mask.
		std::optional<float> allowed_penetration=std::nullopt,   // Defaults to internal collision-world's allowable penetration.
		std::optional<float> max_distance
	);
	*/
}