#include "collision_cast.hpp"

#include "physics_system.hpp"

#include "components/collision_component.hpp"

#include "bullet_util/bullet_util.hpp"

#include <engine/world/world.hpp>

#include <math/math.hpp>
#include <math/bullet.hpp>

#include <bullet/btBulletCollisionCommon.h>
//#include <bullet/btBulletDynamicsCommon.h>

#include <util/variant.hpp>

#include <type_traits>

namespace engine
{
	namespace impl
	{
		/*
			This routine performs a safety check for a zero-distance ray-cast.
			(Returns `false` on 'abort' scenario)
		
			Bullet triggers an assert on vector normalization when attempting this,
			so we check this ahead of bullet and abort the cast operation entirely.
		
			Bullet presumably has this assert to circumvent a divide-by-zero error.
		*/
		static bool _ray_cast_impl_abort_check(const btVector3& from_bt, const btVector3& to_bt)
		{
			// TODO: Determine if this is needed in Release builds.
			// (Probably not, since it's circumventing a `btAssert`)

			auto diff = (to_bt - from_bt); // diff.normalize();

			//return (glm::length(from - to) == 0.0f); // <-- Non-Bullet version.
			return !(diff.fuzzyZero()); // fuzzyZero: (diff.length2() < SIMD_EPSILON* SIMD_EPSILON);
		}

		template <typename CollisionComponentCallback>
		static const btCollisionObject* resolve_self(PhysicsSystem& physics, const RayCastSelf& self, CollisionComponentCallback&& callback)
		{
			const btCollisionObject* self_collision_obj = nullptr;

			util::visit
			(
				self,

				[](const std::monostate&) {},
			
				[&self_collision_obj](const btCollisionObject& c_obj)
				{
					self_collision_obj = &c_obj;
				},

				[&self_collision_obj, &callback](const CollisionComponent& collision)
				{
					self_collision_obj = collision.get_collision_object();

					callback(collision);
				},

				[&physics, &self_collision_obj, &callback](Entity entity)
				{
					auto& world    = physics.get_world();
					auto& registry = world.get_registry();

					const auto* collision = registry.try_get<CollisionComponent>(entity);

					if (!collision)
					{
						return;
					}

					self_collision_obj = collision->get_collision_object();

					callback(*collision);
				}
			);

			return self_collision_obj;
		}

		static const btCollisionObject* resolve_self(PhysicsSystem& physics, const RayCastSelf& self)
		{
			return resolve_self(physics, self, [](const CollisionComponent&) {});
		}

		static math::Vector resolve_origin_vector(PhysicsSystem& physics, const RayCastSelf& self)
		{
			const auto* collision_obj = resolve_self(physics, self);

			assert(collision_obj);

			if (!collision_obj)
			{
				return {};
			}

			const auto& tform = collision_obj->getWorldTransform();
			auto from = math::to_vector(tform.getOrigin());

			return from;
		}

		template <typename CallbackType, typename GroupOptionalType, typename CollisionObjectType>
		static CallbackType& register_collision_filters(CallbackType& callback, GroupOptionalType&& filter_group, GroupOptionalType&& filter_mask, CollisionObjectType* collision_obj=nullptr)
		{
			if (filter_group)
			{
				callback.m_collisionFilterGroup = static_cast<int>(*filter_group);
			}
			else
			{
				int filter_group_fallback = -1; // CollisionGroup::All

				if (collision_obj)
				{
					filter_group_fallback = collision_obj->getBroadphaseHandle()->m_collisionFilterGroup;
				}

				callback.m_collisionFilterGroup = filter_group_fallback;
			}

			if (filter_mask)
			{
				callback.m_collisionFilterMask = static_cast<int>(*filter_mask);
			}
			else
			{
				int filter_mask_fallback = -1; // CollisionGroup::All

				if (collision_obj)
				{
					filter_mask_fallback = collision_obj->getBroadphaseHandle()->m_collisionFilterMask;
				}

				callback.m_collisionFilterMask = filter_mask_fallback;
			}

			return callback;
		}

		// Convex cast implementation; this interface is not stable.
		static std::optional<ConvexCastResult> convex_cast_impl
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
		)
		{
			// Const-cast needed due to non-const pointer used by Bullet,
			// despite no direct modification occurring.
			auto* collision_obj = const_cast<btCollisionObject*>(collision_obj_ptr);

			const auto* collision_world = physics.get_collision_world();

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
				register_collision_filters(callback, filter_group, filter_mask, collision_obj);

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

						callback.m_closestHitFraction,

						// Bullet/native data:
						{
							collision_obj,
							callback.m_hitCollisionObject
						}
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

			const ConvexCastSelf& self,

			const CollisionCastPoint& destination,

			std::optional<CollisionGroup> filter_group,
			std::optional<CollisionGroup> filter_mask,

			std::optional<float> allowed_penetration
		)
		{
			const CollisionComponent* collision       = nullptr;
			const btCollisionObject*  collision_obj   = nullptr;
			const btConvexShape*      collision_shape = nullptr;

			util::visit
			(
				self,

				[&collision](const CollisionComponent& component_inst)
				{
					collision = &component_inst;

					// See below for `collision` logic.
				},

				[&physics, &collision, &collision_obj, &collision_shape](Entity entity)
				{
					auto& world = physics.get_world();
					auto& registry = world.get_registry();

					collision = registry.try_get<CollisionComponent>(entity);

					// See below for `collision` logic.
				},
			
				[&collision_obj, &collision_shape](const CollisionObjectAndConvexShape& bullet_data)
				{
					collision_obj   = &(std::get<0>(bullet_data).get());
					collision_shape = &(std::get<1>(bullet_data).get());
				}
			);

			if (collision)
			{
				collision_shape = collision->peek_convex_shape();
				collision_obj   = collision->get_collision_object();
			}

			assert(collision_shape);
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
		
			// NOTE: This forwarding operation from `collision` is required in the general case,
			// as 'intersections' and 'solids' are grouped together on Bullet's end.
			auto filter_group_bt = static_cast<int>(filter_group.value_or((collision) ? collision->get_group() : CollisionGroup::All));
			auto filter_mask_bt  = static_cast<int>(filter_mask.value_or((collision) ? collision->get_solids() : CollisionGroup::All));

			return convex_cast_impl
			(
				physics,
			
				*collision_shape,
			
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

		std::optional<RayCastResult> ray_cast
		(
			PhysicsSystem& physics,

			const math::Vector& from,
			const math::Vector& to,

			std::optional<CollisionGroup> filter_group,
			std::optional<CollisionGroup> filter_mask,

			const RayCastSelf& self
		)
		{
			const auto from_bt = math::to_bullet_vector(from);
			const auto to_bt   = math::to_bullet_vector(to);
		
			// Safety check for Bullet:
			if (!_ray_cast_impl_abort_check(from_bt, to_bt))
			{
				return std::nullopt;
			}

			const auto* collision_world = physics.get_collision_world();
		
			const auto* self_collision_obj = resolve_self
			(
				physics, self,

				// In the event of a `CollisionComponent` object as `self`, circumvent the
				// default collision-object path for `register_collision_filters` (called below):
				[&](const CollisionComponent& collision)
				{
					// NOTE: This is a workaround for handling interaction-filters, when we
					// know ahead-of-time to exclude them. (i.e. we have a `CollisionComponent` object)

					if (!filter_group.has_value())
					{
						filter_group = collision.get_group();
					}

					if (!filter_mask.has_value())
					{
						filter_mask = collision.get_solids();
					}
				}
			);

			// btCollisionWorld::RayResultCallback
			auto callback = btKinematicClosestNotMeRayResultCallback // btClosestNotMeConvexResultCallback
			(
				self_collision_obj
			);

			register_collision_filters(callback, filter_group, filter_mask, self_collision_obj);

			// TODO: Look into `m_flags` member of `callback`.
			// (Bullet source indicates backface culling, etc.)

			collision_world->rayTest(from_bt, to_bt, callback);

			if (callback.hasHit())
			{
				return RayCastResult
				{
					// General cast result data:
					((self_collision_obj) ? get_entity_from_collision_object(*self_collision_obj) : null), //get_entity_from_collision_object(*callback.m_me), // entity,
					((callback.m_collisionObject) ? get_entity_from_collision_object(*callback.m_collisionObject) : null),

					math::to_vector(callback.m_hitPointWorld),
					math::to_vector(callback.m_hitNormalWorld),

					callback.m_closestHitFraction,

					// Bullet/native data:
					{
						self_collision_obj,
						callback.m_collisionObject
					}
				};
			}

			return std::nullopt;
		}

		std::optional<RayCastResult> directional_ray_cast
		(
			PhysicsSystem& physics,

			const math::Vector& from,
			const math::Vector& to_direction,

			std::optional<float> max_distance,

			std::optional<CollisionGroup> filter_group,
			std::optional<CollisionGroup> filter_mask,

			const RayCastSelf& self
		)
		{
			return ray_cast
			(
				physics,
			
				from,
			
				(to_direction * physics.get_max_ray_distance()),
			
				// Alternative: forced normalization. (Slightly slower)
				//(glm::normalize(to_direction) * physics.get_max_ray_distance()),

				filter_group,
				filter_mask,

				self
			);
		}

		std::optional<RayCastResult> ray_cast_to
		(
			PhysicsSystem& physics,

			const RayCastSelf& self,

			const math::Vector& destination,

			std::optional<CollisionGroup> filter_group,
			std::optional<CollisionGroup> filter_mask
		)
		{
			return ray_cast
			(
				physics,
				resolve_origin_vector(physics, self),
				destination,
				filter_group, filter_mask,
				self
			);
		}

		std::optional<RayCastResult> directional_ray_cast_to
		(
			PhysicsSystem& physics,

			const RayCastSelf& self,

			const math::Vector& direction,
			std::optional<float> max_distance,

			std::optional<CollisionGroup> filter_group,
			std::optional<CollisionGroup> filter_mask
		)
		{
			return directional_ray_cast
			(
				physics,
				resolve_origin_vector(physics, self),
				direction,
				max_distance,
				filter_group, filter_mask,
				self
			);
		}
	}

	std::optional<ConvexCastResult> convex_cast
	(
		PhysicsSystem& physics,

		const btConvexShape& shape,

		const CollisionCastPoint& from,
		const CollisionCastPoint& to,

		CollisionGroup filter_group,
		CollisionGroup filter_mask,

		const btCollisionObject* collision_obj_self_filter,

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

		return impl::convex_cast_impl
		(
			physics,
			
			shape,
			
			from_matrix_bt,
			to_matrix_bt,

			collision_obj_self_filter,
			
			from_position_bt,
			to_position_bt,
			
			static_cast<int>(filter_group),
			static_cast<int>(filter_mask),

			allowed_penetration
		);
	}
}