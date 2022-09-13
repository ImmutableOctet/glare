#include "collision_cast.hpp"

#include "physics.hpp"
#include "collision_component.hpp"

#include "bullet_util/bullet_util.hpp"

#include <engine/world/world.hpp>

#include <math/bullet.hpp>

#include <bullet/btBulletCollisionCommon.h>
//#include <bullet/btBulletDynamicsCommon.h>

#include <util/variant.hpp>

#include <type_traits>

namespace engine
{
	std::optional<ConvexCastResult> convex_cast_impl
	(
		PhysicsSystem& physics,

		const btConvexShape& shape,

		const btTransform& from_matrix_bt,
		const btTransform& to_matrix_bt,

		const btCollisionObject* collision_obj_ptr,

		std::optional<btVector3> from_position_bt,
		std::optional<btVector3> to_position_bt,

		std::optional<int> filter_group,
		std::optional<int> filter_mask,

		std::optional<float> allowed_penetration
	)
	{
		// Const-cast needed due to non-const pointer used by Bullet,
		// despite no direct modification occurring.
		auto* collision_obj = const_cast<btCollisionObject*>(collision_obj_ptr);

		auto* collision_world = physics.get_collision_world();

		assert(collision_world);

		// TODO: Fix this. (should allow for non-centered convex objects; origin to be pulled from `collision_obj`)
		// Currently hard-coded to 'center' origin.
		const auto local_origin = btVector3(0.0f, 0.0f, 0.0f); // collision_obj->getWorldTransform().getOrigin();

		if (!from_position_bt.has_value())
		{
			from_position_bt = (from_matrix_bt * local_origin);
		}

		if (!to_position_bt.has_value())
		{
			to_position_bt = (to_matrix_bt * local_origin);
		}

		auto use_callback = [&]<typename Callback>(Callback&& callback) -> std::optional<ConvexCastResult>
		{
			if (filter_group.has_value())
			{
				callback.m_collisionFilterGroup = *filter_group;
			}
			else
			{
				int filter_group_fallback = -1;

				if (collision_obj)
				{
					filter_group_fallback = collision_obj->getBroadphaseHandle()->m_collisionFilterGroup;
				}

				callback.m_collisionFilterGroup = filter_group_fallback;
			}

			if (filter_mask.has_value())
			{
				callback.m_collisionFilterMask = *filter_mask;
			}
			else
			{
				int filter_mask_fallback = -1;

				if (collision_obj)
				{
					filter_mask_fallback = collision_obj->getBroadphaseHandle()->m_collisionFilterMask;
				}

				callback.m_collisionFilterMask = filter_mask_fallback;
			}

			auto penetration = allowed_penetration.value_or(collision_world->getDispatchInfo().m_allowedCcdPenetration);

			if constexpr (std::is_same_v<Callback, btClosestNotMeConvexResultCallback>)
			{
				callback.m_allowedPenetration = penetration;
			}

			collision_world->convexSweepTest(&shape, from_matrix_bt, to_matrix_bt, callback, penetration);

			if (callback.hasHit())
			{
				return ConvexCastResult
				{
					// General cast result data:
					((collision_obj) ? get_entity_from_collision_object(*collision_obj) : null), //get_entity_from_collision_object(*callback.m_me), // entity,
					((callback.m_hitCollisionObject) ? get_entity_from_collision_object(*callback.m_hitCollisionObject) : null),

					math::to_vector(callback.m_hitPointWorld),
					math::to_vector(callback.m_hitNormalWorld),

					// Bullet/native data:
					{
						collision_obj,
						callback.m_hitCollisionObject
					},

					// Convex-cast specific:
					callback.m_closestHitFraction
				};
			}

			return std::nullopt;
		};

		if (collision_obj)
		{
			return use_callback
			(
				btClosestNotMeConvexResultCallback
				(
					collision_obj,

					*from_position_bt, *to_position_bt,

					physics.get_broadphase()->getOverlappingPairCache(),
					physics.get_collision_dispatcher()
				)
			);
		}
		else
		{
			return use_callback
			(
				btCollisionWorld::ClosestConvexResultCallback
				(
					*from_position_bt, *to_position_bt
				)
			);
		}

		return std::nullopt;
	}

	std::optional<ConvexCastResult> convex_cast_to
	(
		PhysicsSystem& physics,

		Entity entity,
		const CollisionComponent& collision,

		const CollisionCastPoint& destination,

		std::optional<CollisionGroup> filter_group,
		std::optional<CollisionGroup> filter_mask,

		std::optional<float> allowed_penetration
	)
	{
		auto* shape = collision.peek_convex_shape();

		assert(shape);
		
		const auto* collision_obj = collision.get_collision_object();

		assert(collision_obj);

		const auto& from_matrix_bt   = collision_obj->getWorldTransform();
		const auto& from_position_bt = from_matrix_bt.getOrigin();

		// Resolve cast-destination:
		btVector3 to_position_bt;
		btTransform to_matrix_bt;

		util::visit
		(
			destination,

			[&to_position_bt, &to_matrix_bt](const math::Matrix& to)
			{
				to_matrix_bt = math::to_bullet_matrix(to);
				to_position_bt = math::to_bullet_vector(math::get_translation(to));
			},

			[&to_position_bt, &to_matrix_bt, &from_matrix_bt](const math::Vector& to)
			{
				to_position_bt = math::to_bullet_vector(to);

				to_matrix_bt = btTransform(from_matrix_bt.getBasis(), to_position_bt);

				// Alternative: no orientation at destination.
				//to_matrix_bt = btTransform(btMatrix3x3::getIdentity(), to_position_bt);
			}
		);

		// NOTE: Alternatively, we could simply forward `std::nullopt` values and grab these filters back from Bullet.
		auto filter_group_bt = static_cast<int>(filter_group.value_or(collision.get_group()));
		auto filter_mask_bt  = static_cast<int>(filter_mask.value_or(collision.get_solids()));

		return convex_cast_impl
		(
			physics,
			
			*shape,
			
			from_matrix_bt,
			to_matrix_bt,

			collision_obj,
			
			from_position_bt,
			to_position_bt,
			
			filter_group_bt,
			filter_mask_bt,

			allowed_penetration
		);
	}

	// Shorthand overload.
	std::optional<ConvexCastResult> convex_cast_to
	(
		PhysicsSystem& physics,

		Entity entity,

		const CollisionCastPoint& destination,

		std::optional<CollisionGroup> filter_group,
		std::optional<CollisionGroup> filter_mask,
		std::optional<float> allowed_penetration
	)
	{
		auto& world = physics.get_world();
		auto& registry = world.get_registry();

		const auto* collision = registry.try_get<CollisionComponent>(entity);

		assert(collision);

		if (!collision)
		{
			return std::nullopt;
		}

		return convex_cast_to
		(
			physics,

			entity,
			*collision,
			
			destination,
			
			filter_group,
			filter_mask,
			
			allowed_penetration
		);
	}

	std::optional<ConvexCastResult> convex_cast
	(
		PhysicsSystem& physics,

		const btConvexShape& shape,

		const CollisionCastPoint& from,
		const CollisionCastPoint& to,

		CollisionGroup filter_group,
		CollisionGroup filter_mask,

		std::optional<float> allowed_penetration
	)
	{
		// Resolve cast-source:
		btVector3 from_position_bt;
		btTransform from_matrix_bt;

		util::visit
		(
			from,

			[&from_position_bt, &from_matrix_bt](const math::Matrix& from)
			{
				from_matrix_bt = math::to_bullet_matrix(from);
				from_position_bt = math::to_bullet_vector(math::get_translation(from));
			},

			[&from_position_bt, &from_matrix_bt](const math::Vector& from)
			{
				from_position_bt = math::to_bullet_vector(from);
				from_matrix_bt = btTransform(btMatrix3x3::getIdentity(), from_position_bt);
			}
		);

		// Resolve cast-destination:
		btVector3 to_position_bt;
		btTransform to_matrix_bt;

		util::visit
		(
			to,

			[&to_position_bt, &to_matrix_bt](const math::Matrix& to)
			{
				to_matrix_bt = math::to_bullet_matrix(to);
				to_position_bt = math::to_bullet_vector(math::get_translation(to));
			},

			[&to_position_bt, &to_matrix_bt, &from_matrix_bt](const math::Vector& to)
			{
				to_position_bt = math::to_bullet_vector(to);

				to_matrix_bt = btTransform(from_matrix_bt.getBasis(), to_position_bt);

				// Alternative: no orientation at destination.
				//to_matrix_bt = btTransform(btMatrix3x3::getIdentity(), to_position_bt);
			}
		);

		return convex_cast_impl
		(
			physics,
			
			shape,
			
			from_matrix_bt,
			to_matrix_bt,

			nullptr,
			
			from_position_bt,
			to_position_bt,
			
			static_cast<int>(filter_group),
			static_cast<int>(filter_mask),

			allowed_penetration
		);
	}
}