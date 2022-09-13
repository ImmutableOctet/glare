#include "collision_cast.hpp"

#include "physics.hpp"
#include "collision_component.hpp"

#include "bullet_util/bullet_util.hpp"

#include <engine/world/world.hpp>

#include <math/bullet.hpp>

#include <bullet/btBulletCollisionCommon.h>
//#include <bullet/btBulletDynamicsCommon.h>

#include <util/variant.hpp>

namespace engine
{
	std::optional<ConvexCastResult> convex_cast_impl
	(
		PhysicsSystem& physics,

		const btCollisionObject& collision_obj_ref,
		const btConvexShape& shape,

		const btTransform& from_matrix_bt,
		const btTransform& to_matrix_bt,

		std::optional<btVector3> from_position_bt,
		std::optional<btVector3> to_position_bt,

		std::optional<int> filter_group,
		std::optional<int> filter_mask,

		std::optional<float> allowed_penetration
	)
	{
		// Const-cast needed due to non-const pointer used by Bullet,
		// despite no direct modification occurring.
		auto& collision_obj = const_cast<btCollisionObject&>(collision_obj_ref);

		auto* collision_world = physics.get_collision_world();

		assert(collision_world);

		if (!allowed_penetration.has_value())
		{
			allowed_penetration = collision_world->getDispatchInfo().m_allowedCcdPenetration;
		}

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

		auto callback = btClosestNotMeConvexResultCallback // btCollisionWorld::ClosestConvexResultCallback
		(
			&collision_obj,

			*from_position_bt, *to_position_bt,

			physics.get_broadphase()->getOverlappingPairCache(),
			physics.get_collision_dispatcher()
		);

		callback.m_collisionFilterGroup = filter_group.value_or(collision_obj.getBroadphaseHandle()->m_collisionFilterGroup); // -1;
		callback.m_collisionFilterMask  = filter_mask.value_or(collision_obj.getBroadphaseHandle()->m_collisionFilterMask); // -1;

		callback.m_allowedPenetration = *allowed_penetration;

		collision_world->convexSweepTest(&shape, from_matrix_bt, to_matrix_bt, callback, *allowed_penetration);

		if (callback.hasHit())
		{
			return ConvexCastResult
			{
				// General cast result data:
				get_entity_from_collision_object(*callback.m_me), // get_entity_from_collision_object(collision_obj), // entity,
				get_entity_from_collision_object(*callback.m_hitCollisionObject),

				math::to_vector(callback.m_hitPointWorld),
				math::to_vector(callback.m_hitNormalWorld),

				// Bullet/native data:
				{
					&collision_obj,
					callback.m_hitCollisionObject
				},

				// Convex-cast specific:
				callback.m_closestHitFraction
			};
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
			
			*collision_obj,
			*shape,
			
			from_matrix_bt,
			to_matrix_bt,
			
			from_position_bt,
			to_position_bt,
			
			filter_group_bt,
			filter_mask_bt,

			allowed_penetration
		);
	}

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
}