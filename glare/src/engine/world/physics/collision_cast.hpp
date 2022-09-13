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
	using CollisionCastPoint = std::variant<math::Matrix, math::Vector>;

	// Convex cast implementation; this inteface is non-stable.
	std::optional<ConvexCastResult> convex_cast_impl
	(
		PhysicsSystem& physics,
		
		const btConvexShape& shape,

		const btTransform& from_matrix_bt,
		const btTransform& to_matrix_bt,

		const btCollisionObject* collision_obj_ptr=nullptr,

		std::optional<btVector3> from_position_bt=std::nullopt,
		std::optional<btVector3> to_position_bt=std::nullopt,

		std::optional<int> filter_group=std::nullopt,         // Defaults to `collision_obj_ref`'s filter.
		std::optional<int> filter_mask=std::nullopt,          // Defaults to `collision_obj_ref`'s mask.

		std::optional<float> allowed_penetration=std::nullopt // Defaults to the collision-world's allowable penetration.
	);
	
	// Performs a cast using a convex object (`entity`) from its current position to the specified position in world-space.
	// If a hit is detected, this function will return a `ConvexCastResult` object, otherwise, the return value will be `std::nullopt`.
	std::optional<ConvexCastResult> convex_cast_to
	(
		PhysicsSystem& physics,

		Entity entity,
		const CollisionComponent& collision,

		const CollisionCastPoint& destination,

		std::optional<CollisionGroup> filter_group=std::nullopt, // Defaults to `collision` object's filter.
		std::optional<CollisionGroup> filter_mask=std::nullopt,  // Defaults to `collision` object's mask.

		std::optional<float> allowed_penetration=std::nullopt    // Defaults to internal collision-world's allowable penetration.
	);

	// Shorthand overload: See main overload for details.
	std::optional<ConvexCastResult> convex_cast_to
	(
		PhysicsSystem& physics,

		Entity entity,

		const CollisionCastPoint& destination,

		std::optional<CollisionGroup> filter_group=std::nullopt, // Defaults to `collision` object's filter.
		std::optional<CollisionGroup> filter_mask=std::nullopt,  // Defaults to `collision` object's mask.
		std::optional<float> allowed_penetration=std::nullopt    // Defaults to internal collision-world's allowable penetration.
	);

	// Performs a convex-cast using `shape` from the location
	// specified (`from`) to the destination specified (`to`).
	std::optional<ConvexCastResult> convex_cast
	(
		PhysicsSystem& physics,

		const btConvexShape& shape,

		const CollisionCastPoint& from,
		const CollisionCastPoint& to,

		CollisionGroup filter_group=CollisionGroup::All,
		CollisionGroup filter_mask=CollisionGroup::All,

		std::optional<float> allowed_penetration=std::nullopt
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