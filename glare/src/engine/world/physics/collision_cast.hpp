#pragma once

#include "collision_cast_result.hpp"
#include "collision_group.hpp"
#include "directional_ray.hpp"

#include <engine/types.hpp>

#include <math/types.hpp>

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

	namespace impl
	{
		// TODO: Refactor usage of `std::variant`.
		using RayCastSelf = std::variant
		<
			std::monostate,
			std::reference_wrapper<const btCollisionObject>,
			std::reference_wrapper<const CollisionComponent>,
			Entity
		>;

		// TODO: Refactor usage of `std::variant`.
		using ConvexCastSelf = std::variant
		<
			std::reference_wrapper<const CollisionComponent>,
			Entity,
			CollisionObjectAndConvexShape
		>;

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

			const btCollisionObject* collision_obj_self=nullptr,

			std::optional<float> allowed_penetration=std::nullopt,
			std::optional<float> skin_width=std::nullopt,

			const math::Vector& shape_offset={},
			bool apply_shape_offset=true,
			bool apply_shape_size_to_offset=false
		);

		std::optional<ConvexCastResult> convex_cast_to
		(
			PhysicsSystem& physics,

			const ConvexCastSelf& self,

			const CollisionCastPoint& destination,

			std::optional<CollisionGroup> filter_group=std::nullopt,
			std::optional<CollisionGroup> filter_mask=std::nullopt,

			std::optional<float> allowed_penetration=std::nullopt,
			std::optional<float> skin_width=std::nullopt,

			const math::Vector& shape_offset={},
			bool apply_shape_offset=true,
			bool apply_shape_size_to_offset=false
		);

		std::optional<RayCastResult> ray_cast
		(
			PhysicsSystem& physics,

			const math::Vector& from,
			const math::Vector& to,

			std::optional<CollisionGroup> filter_group=std::nullopt,
			std::optional<CollisionGroup> filter_mask=std::nullopt,

			const RayCastSelf& self={}
		);

		std::optional<RayCastResult> directional_ray_cast
		(
			PhysicsSystem& physics,

			const math::Vector& from,
			const math::Vector& to_direction,

			std::optional<float> max_distance=std::nullopt,

			std::optional<CollisionGroup> filter_group=std::nullopt,
			std::optional<CollisionGroup> filter_mask=std::nullopt,

			const RayCastSelf& self={}
		);

		std::optional<RayCastResult> ray_cast_to
		(
			PhysicsSystem& physics,

			const RayCastSelf& self,

			const math::Vector& destination,

			std::optional<CollisionGroup> filter_group=std::nullopt,
			std::optional<CollisionGroup> filter_mask=std::nullopt
		);

		std::optional<RayCastResult> directional_ray_cast_to
		(
			PhysicsSystem& physics,

			const RayCastSelf& self,

			const math::Vector& direction,
			std::optional<float> max_distance=std::nullopt,

			std::optional<CollisionGroup> filter_group=std::nullopt,
			std::optional<CollisionGroup> filter_mask=std::nullopt
		);
	}

	// Performs a cast using a convex object (`entity`) from its current position to the specified position in world-space.
	// If a hit is detected, this function will return a `ConvexCastResult` object, otherwise, the return value will be `std::nullopt`.
	// By convention, the closest hit is always used.
	template <typename SelfType>
	std::optional<ConvexCastResult> convex_cast_to
	(
		PhysicsSystem& physics,

		const SelfType& self,

		const CollisionCastPoint& destination,

		std::optional<CollisionGroup> filter_group=std::nullopt, // Defaults to `collision` object's group.
		std::optional<CollisionGroup> filter_mask=std::nullopt,  // Defaults to `collision` object's mask.

		std::optional<float> allowed_penetration=std::nullopt,   // Defaults to internal collision-world's allowable penetration.
		std::optional<float> skin_width=std::nullopt             // Defaults to nothing.
	)
	{
		return impl::convex_cast_to(physics, self, destination, filter_group, filter_mask, allowed_penetration, skin_width);
	}

	// Performs a ray-cast between two points, returning the closest hit detected.
	// If no hit was found, this function will return `std::nullopt`.
	// If `self` is specified, that collision-object will be explicitly ignored when determining the closest hit.
	template <typename SelfType>
	std::optional<RayCastResult> ray_cast
	(
		PhysicsSystem& physics,

		const SelfType& self,

		const math::Vector& from,
		const math::Vector& to,

		std::optional<CollisionGroup> filter_group=std::nullopt, // Defaults to filter resolved from `self`, if available. (uses `CollisionGroup::All` otherwise)
		std::optional<CollisionGroup> filter_mask=std::nullopt  // Defaults to mask resolved from `self`, if available. (uses `CollisionGroup::All` otherwise)
	)
	{
		return impl::ray_cast(physics, from, to, filter_group, filter_mask, self);
	}

	// Performs a ray-cast between two points, returning the closest hit detected.
	// If no hit was found, this function will return `std::nullopt`.
	inline std::optional<RayCastResult> ray_cast
	(
		PhysicsSystem& physics,

		const math::Vector& from,
		const math::Vector& to,

		std::optional<CollisionGroup> filter_group=std::nullopt,
		std::optional<CollisionGroup> filter_mask=std::nullopt
	)
	{
		return impl::ray_cast(physics, from, to, filter_group, filter_mask);
	}

	// Similar to `ray_cast`, but allows for a normalized `to_direction` vector, instead of an exact end-position.
	// This can be useful for things like hitscan weapons, speculative floor-detection, etc.
	// Like `ray_cast`, `self` can be specified to ignore a specific entity/collision-object.
	template <typename SelfType>
	std::optional<RayCastResult> directional_ray_cast
	(
		PhysicsSystem& physics,

		const SelfType& self,

		const math::Vector& from,
		const math::Vector& to_direction,

		std::optional<float> max_distance=std::nullopt,          // If no maximum distance is supplied, then the `physics` object's configured max-distance is used.

		std::optional<CollisionGroup> filter_group=std::nullopt, // Defaults to filter resolved from `self`, if available.
		std::optional<CollisionGroup> filter_mask=std::nullopt   // Defaults to mask resolved from `self`, if available.
	)
	{
		return impl::directional_ray_cast(physics, from, to_direction, max_distance, filter_group, filter_mask, self);
	}

	/*
	// Convenience overload allowing for `DirectionalRay` as input.
	// (Disabled for now, due to reflection name conflicts)
	template <typename SelfType>
	std::optional<RayCastResult> directional_ray_cast
	(
		PhysicsSystem& physics,

		const SelfType& self,

		const DirectionalRay& ray,

		std::optional<float> max_distance=std::nullopt,          // If no maximum distance is supplied, then the `physics` object's configured max-distance is used.

		std::optional<CollisionGroup> filter_group=std::nullopt, // Defaults to filter resolved from `self`, if available.
		std::optional<CollisionGroup> filter_mask=std::nullopt   // Defaults to mask resolved from `self`, if available.
	)
	{
		return impl::directional_ray_cast(physics, ray.origin, ray.direction, max_distance, filter_group, filter_mask, self);
	}
	*/

	// Similar to `ray_cast`, but allows for a normalized `to_direction` vector, instead of an exact end-position.
	// This can be useful for things like hitscan weapons, speculative floor-detection, etc.
	inline std::optional<RayCastResult> directional_ray_cast
	(
		PhysicsSystem& physics,

		const math::Vector& from,
		const math::Vector& to_direction,

		std::optional<float> max_distance=std::nullopt, // If no maximum distance is supplied, then the `physics` object's configured max-distance is used.

		std::optional<CollisionGroup> filter_group=std::nullopt,
		std::optional<CollisionGroup> filter_mask=std::nullopt
	)
	{
		return impl::directional_ray_cast(physics, from, to_direction, max_distance, filter_group, filter_mask);
	}

	/*
	// Convenience overload allowing for `DirectionalRay` as input.
	// (Disabled for now, due to reflection name conflicts)
	inline std::optional<RayCastResult> directional_ray_cast
	(
		PhysicsSystem& physics,

		const DirectionalRay& ray,

		std::optional<float> max_distance=std::nullopt, // If no maximum distance is supplied, then the `physics` object's configured max-distance is used.

		std::optional<CollisionGroup> filter_group=std::nullopt,
		std::optional<CollisionGroup> filter_mask=std::nullopt
	)
	{
		return impl::directional_ray_cast(physics, ray.origin, ray.direction, max_distance, filter_group, filter_mask);
	}
	*/

	// Performs a ray-cast for `self` from its current position to `destination`.
	// See also: `ray_cast` for general-purpose ray-casting.
	template <typename SelfType>
	std::optional<RayCastResult> ray_cast_to
	(
		PhysicsSystem& physics,

		const SelfType& self,

		const math::Vector& destination,

		std::optional<CollisionGroup> filter_group=std::nullopt, // Defaults to filter resolved from `self`.
		std::optional<CollisionGroup> filter_mask=std::nullopt  // Defaults to mask resolved from `self`.
	)
	{
		return impl::ray_cast_to(physics, self, destination, filter_group, filter_mask);
	}

	// Performs a ray-cast for `self` from its current position to the `direction` specified.
	// See also: `directional_ray_cast` for general-purpose ray-casting.
	template <typename SelfType>
	std::optional<RayCastResult> directional_ray_cast_to
	(
		PhysicsSystem& physics,

		const SelfType& self,

		const math::Vector& direction,
		std::optional<float> max_distance=std::nullopt,          // If no maximum distance is supplied, then the `physics` object's configured max-distance is used.

		std::optional<CollisionGroup> filter_group=std::nullopt, // Defaults to filter resolved from `self`.
		std::optional<CollisionGroup> filter_mask=std::nullopt  // Defaults to mask resolved from `self`.
	)
	{
		return impl::directional_ray_cast_to(physics, self, direction, max_distance, filter_group, filter_mask);
	}

	using impl::convex_cast;
}