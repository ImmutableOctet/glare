#pragma once

#include "collision_cast_result.hpp"
#include "collision_group.hpp"

#include <engine/types.hpp>
#include <math/math.hpp>

#include <optional>
#include <variant>
#include <tuple>
#include <functional>

#include <bullet/LinearMath/btVector3.h>
#include <bullet/LinearMath/btTransform.h>

//class btVector3;
//class btTransform;

class btConvexShape;
class btCollisionObject;

/*
	TODO:
		* Look into `btTriangleRaycastCallback` and `btTriangleConvexcastCallback`.
		(May be needed for detecting vertex attributes; e.g. material/texture)

		* Implement multi-hit ray-cast using callable,
		rather than Bullet's default allocation-based approach.
*/

namespace engine
{
	class PhysicsSystem;
	struct CollisionComponent;

	// Destination for a cast operation; can either be a world-space position or a matrix.
	using CollisionCastPoint = std::variant<math::Matrix, math::Vector>;

	using CollisionObjectAndConvexShape = std::tuple
	<
		std::reference_wrapper<const btCollisionObject>,
		std::reference_wrapper<const btConvexShape>
	>;

	using RayCastSelf = std::variant
	<
		std::monostate,
		std::reference_wrapper<const btCollisionObject>,
		std::reference_wrapper<const CollisionComponent>,
		Entity
	>;

	using ConvexCastSelf = std::variant
	<
		std::reference_wrapper<const CollisionComponent>,
		Entity,
		CollisionObjectAndConvexShape
	>;

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
	// By convention, the closest hit is always used.
	std::optional<ConvexCastResult> convex_cast_to
	(
		PhysicsSystem& physics,

		const ConvexCastSelf& self,

		const CollisionCastPoint& destination,

		std::optional<CollisionGroup> filter_group=std::nullopt, // Defaults to `collision` object's group.
		std::optional<CollisionGroup> filter_mask=std::nullopt,  // Defaults to `collision` object's mask.

		std::optional<float> allowed_penetration=std::nullopt    // Defaults to internal collision-world's allowable penetration.
	);

	/*
		Performs a convex-cast using `shape` from the location
		specified (`from`) to the destination specified (`to`).

		By convention, the closest hit is always used.

		See also: `convex_cast_to` for established collision-objects
		with well defined starting positions/transforms.
	*/
	std::optional<ConvexCastResult> convex_cast
	(
		PhysicsSystem& physics,

		const btConvexShape& shape,

		const CollisionCastPoint& from,
		const CollisionCastPoint& to,

		CollisionGroup filter_group=CollisionGroup::All,
		CollisionGroup filter_mask=CollisionGroup::All,

		const btCollisionObject* collision_obj_self_filter=nullptr,

		std::optional<float> allowed_penetration=std::nullopt
	);

	// Performs a ray-cast between two points, returning the closest hit detected.
	// If no hit was found, this function will return `std::nullopt`.
	// If `self` is specified, that collision-object will be explicitly ignored when determining the closest hit.
	std::optional<RayCastResult> ray_cast
	(
		PhysicsSystem& physics,

		const math::Vector& from,
		const math::Vector& to,

		std::optional<CollisionGroup> filter_group=std::nullopt, // Defaults to filter resolved from `self`, if available. (uses `CollisionGroup::All` otherwise)
		std::optional<CollisionGroup> filter_mask=std::nullopt,  // Defaults to mask resolved from `self`, if available. (uses `CollisionGroup::All` otherwise)

		const RayCastSelf& self=std::monostate{}
	);

	// Similar to `ray_cast`, but allows for a normalized `to_direction` vector, instead of an exact end-position.
	// This can be useful for things like hitscan weapons, speculative floor-detection, etc.
	// Like `ray_cast`, `self` can be specified to ignore a specific entity/collision-object.
	std::optional<RayCastResult> ray_cast_directional
	(
		PhysicsSystem& physics,

		const math::Vector& from,
		const math::Vector& to_direction,

		std::optional<float> max_distance=std::nullopt,          // If no maximum distance is supplied, then the `physics` object's configured max-distance is used.

		std::optional<CollisionGroup> filter_group=std::nullopt, // Defaults to filter resolved from `self`, if available.
		std::optional<CollisionGroup> filter_mask=std::nullopt,  // Defaults to mask resolved from `self`, if available.

		const RayCastSelf& self=std::monostate{}
	);


	// Performs a ray-cast for `self` from its current position to `destination`.
	// See also: `ray_cast`
	std::optional<RayCastResult> ray_cast_to
	(
		PhysicsSystem& physics,

		const RayCastSelf& self,

		const math::Vector& destination,

		std::optional<CollisionGroup> filter_group=std::nullopt, // Defaults to filter resolved from `self`.
		std::optional<CollisionGroup> filter_mask=std::nullopt  // Defaults to mask resolved from `self`.
	);
}