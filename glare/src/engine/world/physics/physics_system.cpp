#include "physics_system.hpp"

#include "collision.hpp"

#include "components/collision_component.hpp"

#include "collision_cast.hpp"
#include "collision_motion_state.hpp"
#include "kinematic_resolution_config.hpp"

#include "collision_events.hpp"

#include "bullet_util/bullet_util.hpp"

#include <engine/world/world.hpp>
#include <engine/world/world_events.hpp>

#include <engine/components/relationship_component.hpp>
#include <engine/transform.hpp>

#include <math/bullet.hpp>
#include <util/variant.hpp>
#include <util/optional.hpp>

// See notes in header for these includes.
/*
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
*/

#include <glm/glm.hpp>

#include <variant>
#include <cmath>
#include <type_traits>

// Debugging related:
#include <engine/world/entity.hpp>
#include <engine/world/graphics_entity.hpp>

namespace app
{
	struct Graphics;
}

namespace engine
{
	// PhysicsSystem:
	PhysicsSystem::PhysicsSystem(World& world)
	:
		WorldSystem(world),

		collision_configuration(std::make_unique<btDefaultCollisionConfiguration>()),
		collision_dispatcher(std::make_unique<btCollisionDispatcher>(collision_configuration.get())),
		broadphase(std::make_unique<btDbvtBroadphase>()),

		//solver(std::make_unique<>()),
		solver(std::make_unique<btSequentialImpulseConstraintSolver>()),

		//collision_world(std::make_unique<btSimpleDynamicsWorld>(collision_dispatcher.get(), broadphase.get(), solver.get(), collision_configuration.get()))
		collision_world(std::make_unique<btDiscreteDynamicsWorld>(collision_dispatcher.get(), broadphase.get(), solver.get(), collision_configuration.get()))
		//collision_world(std::make_unique<btCollisionWorld>(collision_dispatcher.get(), broadphase.get(), collision_configuration.get()))
	{
		set_physics_gravity(world.get_gravity());

		//collision_world->setSynchronizeAllMotionStates(true);

		world.subscribe(*this);
	}

	/*
	PhysicsSystem::~PhysicsSystem()
	{
		unsubscribe(world);
	}
	*/

	void PhysicsSystem::on_subscribe(World& world)
	{
		auto& registry = world.get_registry();

		// Standard engine events:
		world.register_event<OnTransformChanged, &PhysicsSystem::on_transform_change>(*this);
		world.register_event<OnGravityChanged, &PhysicsSystem::on_gravity_change>(*this);

		// TODO: Look into circumventing named functions for event aliasing.
		// (Cast + template instantiation of `forward_collision_event`)
		world.register_event<OnSurfaceContact, &PhysicsSystem::on_surface_contact>(*this);
		world.register_event<OnIntersection, &PhysicsSystem::on_intersection>(*this);

		// Core registry events:
		registry.on_construct<CollisionComponent>().connect<&PhysicsSystem::on_create_collider>(*this);
		registry.on_destroy<CollisionComponent>().connect<&PhysicsSystem::on_destroy_collider>(*this);
	}

	// TODO: We need to look at `OnTransformChanged`/`on_transform_change` and how it relates to the collision side of this routine.
	void PhysicsSystem::on_update(World& world, float delta)
	{
		update_collision_world(delta);
	}

	void PhysicsSystem::on_render(World& world, app::Graphics& graphics)
	{
		collision_world->debugDrawWorld();
	}

	void PhysicsSystem::register_debug_drawer(btIDebugDraw& dbg_draw) // BulletDebugDrawer&
	{
		collision_world->setDebugDrawer(&dbg_draw);
	}

	void PhysicsSystem::unregister_debug_drawer()
	{
		collision_world->setDebugDrawer(nullptr);
	}

	void PhysicsSystem::update_collision_world(float delta)
	{
		auto& registry = world.get_registry();

		/*
		collision_world->updateAabbs();
		collision_world->computeOverlappingPairs();
		collision_world->performDiscreteCollisionDetection();
		*/

		collision_world->stepSimulation(delta);

		#if defined(ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL) && (ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL == 1)
			retrieve_bullet_transforms();
		#endif

		handle_intersections();
	}

	void PhysicsSystem::handle_intersections(bool check_resolution_flags)
	{
		auto& dispatcher = *collision_dispatcher;

		auto manifold_count = dispatcher.getNumManifolds();

		if (manifold_count == 0)
		{
			return;
		}

		for (auto i = 0; i < manifold_count; i++)
		{
			const auto* contact = dispatcher.getManifoldByIndexInternal(i);

			const auto contact_count = contact->getNumContacts();

			auto* a = contact->getBody0();
			auto* b = contact->getBody1();

			auto a_ent = get_entity_from_collision_object(*a);
			auto b_ent = get_entity_from_collision_object(*b);

			// NOTE: Need to validate that this behavior is intended/consistent from Bullet.
			// Notify listeners that `a` has entered `b`'s axis-aligned bounding box.
			world.queue_event
			(
				OnAABBOverlap
				{
					.entity = a_ent,
					.bounding = b_ent,
					
					.number_of_contacts = contact_count,

					.native =
					{
						.entity_collision_obj   = a,
						.bounding_collision_obj = b
					}
				}
			);

			// If there are no contacts between the two objects,
			// don't bother checking for intersections.
			if (contact_count == 0)
			{
				continue;
			}

			// Average accumulators:

			// NOTE: We're using Bullet's vector types for efficiency purposes here.
			btVector3 avg_world_hit_position = { 0.0f, 0.0f, 0.0f };
			btVector3 avg_hit_normal         = { 0.0f, 0.0f, 0.0f };
			float     avg_penetration_depth  = 0.0f;

			// The positional-correction to be applied to `a`.
			math::Vector correction = {};

			for (int j = 0; j < contact_count; j++)
			{
				const auto& contact_point = contact->getContactPoint(j);

				const auto distance = contact_point.getDistance(); // Equivalent to: glm::length((b_point - a_point));

				// Accumulate sum of values for average calculation:
				avg_world_hit_position += contact_point.getPositionWorldOnA(); // contact_point.getPositionWorldOnB();
				avg_hit_normal         += contact_point.m_normalWorldOnB;
				avg_penetration_depth  += distance;

				// NOTE:
				// A negative distance indicates moving towards the object, whereas a positive
				// penetration distance indicates moving away-from/out-of the object.
				if (distance < 0.0f)
				{
					const auto normal = math::to_vector(contact_point.m_normalWorldOnB); // glm::normalize(...);
					const auto length = -distance;

					correction += (normal * length);
				}
			}

			// Average the correction-amount by the number of contacts:
			const auto contact_count_f = static_cast<float>(contact_count);

			avg_world_hit_position /= contact_count_f;
			avg_hit_normal         /= contact_count_f;
			avg_penetration_depth  /= contact_count_f;

			// Apply correction, if applicable:

			// An object must be flagged as kinematic in order to resolve its intersections here:
			bool is_kinematic = (a->getCollisionFlags() & btCollisionObject::CollisionFlags::CF_KINEMATIC_OBJECT);

			if (!is_kinematic)
			{
				continue;
			}

			auto& registry = world.get_registry();

			const auto* a_collision = registry.try_get<CollisionComponent>(a_ent);

			if (!a_collision)
			{
				continue;
			}

			int b_group = b->getBroadphaseHandle()->m_collisionFilterGroup;

			if (b_group & static_cast<int>(a_collision->get_interactions()))
			{
				// Notify listeners that this `entity` interacted with `hit_entity` via intersection.
 				world.queue_event
				(
					OnInteractionIntersection
					{
						.collision =
						{
							.a = a_ent,
							.b = b_ent,

							.a_position = math::to_vector(a->getWorldTransform().getOrigin()),
							.b_position = math::to_vector(b->getWorldTransform().getOrigin()),

							.position    = math::to_vector(avg_world_hit_position),
							.normal      = math::to_vector(avg_hit_normal),
							.penetration = avg_penetration_depth,

							.native =
							{
								.a_object = a,
								.b_object = b
							},

							.contact_type = ContactType::Interaction // ContactType::Intersection
						}
					}
				);

				// NOTE: Since 'interaction' collision groups and 'solid' collision
				// groups are not mutually exclusive, we do not short-circuit here.
			}

			// Ensure `b_group` is considered solid; if not, continue to the next loop iteration.
			if (!(b_group & static_cast<int>(a_collision->get_solids())))
			{
				continue;
			}

			auto kinematic_resolution = a_collision->get_kinematic_resolution();

			if (kinematic_resolution.has_value())
			{
				// TODO: Look into whether we should keep this check.
				if (!kinematic_resolution->accepts_influence)
				{
					continue;
				}

				if (!kinematic_resolution->resolve_intersections) // (kinematic_resolution->cast_method != CollisionCastMethod::None)
				{
					continue;
				}
			}

			correction /= contact_count_f;

			auto a_tranform = world.get_transform(a_ent);
			//auto a_position = a_tranform.get_position();

			a_tranform.move(correction);

			// Notify listeners that this `entity` contacted `hit_entity`'s surface.
 			world.queue_event
			(
				OnIntersection
				{
					.collision =
					{
						.a = a_ent,
						.b = b_ent,

						// NOTE: We could probably 'optimize'/change this by storing `a_transform.get_position()`
						// prior to applying the correction, but this is more consistent with `b`'s method, etc.
						.a_position = math::to_vector(a->getWorldTransform().getOrigin()),
						.b_position = math::to_vector(b->getWorldTransform().getOrigin()),

						.position    = math::to_vector(avg_world_hit_position),
						.normal      = math::to_vector(avg_hit_normal),
						.penetration = avg_penetration_depth,

						.native =
						{
							.a_object = a,
							.b_object = b
						},

						.contact_type = ContactType::Intersection
					},
					
					.correction = correction
				}
			);
		}
	}

	void PhysicsSystem::on_gravity_change(const OnGravityChanged& gravity)
	{
		// Forward the world's new gravity to Bullet.
		set_physics_gravity(gravity.new_gravity);
	}

	void PhysicsSystem::on_transform_change(const OnTransformChanged& tform_change)
	{
		auto entity = tform_change.entity;

		auto& registry = world.get_registry();

		auto* col = registry.try_get<CollisionComponent>(entity);

		if (!col)
		{
			return;
		}

		auto* motion_state = col->get_motion_state();

		if (motion_state)
		{
			#if defined(ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL) && (ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL == 1)
				// Handle motion-state submission/update to Bullet:
				// We perform this check before submission in order to reduce the overhead of calling `get_matrix`.
				if (motion_state->can_submit_to_bullet())
				{
					auto transform = world.get_transform(entity);

					motion_state->submit_to_bullet(transform.get_matrix());
				}
			#else
				/*
				// Disabled for now; Bullet will call into `motion_state`'s
				// set `setWorldTransform` method every time, causing another
				// `OnTransformChanged` event-trigger continuously.
				if (motion_state)
				{
					auto* rigid_body = col->get_rigid_body();

					if (rigid_body)
					{
						collision_world->synchronizeSingleMotionState(rigid_body);
					}
				}
				*/
			#endif
		}
		else
		{
			// Kinematic logic:
			auto* collision_obj = col->get_collision_object();

			if (!collision_obj)
			{
				return;
			}

			auto transform = world.get_transform(entity);

			if (col->is_active() && col->is_kinematic())
			{
				handle_transform_resolution(entity, *col, *collision_obj, transform);
			}

			//collision_world->convexSweepTest();
			//collision_world->contactTest();

			update_collision_object_transform(*collision_obj, transform.get_matrix());

			// Debugging related:
			//auto t = col->get_collision_object()->getWorldTransform();
			//print("Position is now: {}", math::to_vector(t.getOrigin()));
		}
	}

	// TODO: Look into the pros and cons of refactoring this into multiple internal functions.
	void PhysicsSystem::handle_transform_resolution
	(
		Entity entity, CollisionComponent& collision,
		btCollisionObject& collision_obj,
		Transform& transform
	)
	{
		auto& registry = world.get_registry(); // Used below.

		const auto kinematic_resolution_opt = collision.get_kinematic_resolution();

		if (!kinematic_resolution_opt.has_value())
		{
			return;
		}

		const auto& kinematic_resolution = *kinematic_resolution_opt;

		// TODO: Look into reducing number of casts performed via delta length-check. (Optimization opportunity)
		//auto position_delta = glm::length(math::to_vector(to_position) - math::to_vector(from_position));

		auto kinematic_resolve = [&](const auto& result)
		{
			const auto* hit_object = result->native.hit_object;
			const auto  hit_entity = result->hit_entity;

			//auto fraction = callback.m_closestHitFraction;

			const auto& hit_normal = result->hit_normal;
			const auto& hit_point_in_world = result->hit_position;

			//world.set_position(create_cube(world), hit_point_in_world);

			auto old_position = math::to_vector(collision_obj.getWorldTransform().getOrigin());
			auto new_position = transform.get_position();

			const auto impact_velocity = (new_position - old_position);
			const auto penetration = (new_position - hit_point_in_world);

			// The influence applied to the `hit_entity`, if any.
			// This variable is modified in `handle_influence_on_hit_entity`
			// and optionally read in `adjust_entity_position`.
			math::Vector influence = {};

			const auto* hit_collision = registry.try_get<CollisionComponent>(hit_entity);

			auto hit_resolution = util::optional_inspect<KinematicResolutionConfig>
			(
				hit_collision,
				[](const auto& hit_collision){ return hit_collision->get_kinematic_resolution(); }
			);

			bool apply_influence  = true;
			bool apply_correction = true;

			// TODO: Integrate this flag into collision-events in some way.
			// 
			// NOTE: We update this value if `child_enumeration_required` is true,
			// but we otherwise assume false to be the correct value.
			bool hit_entity_is_child = false;

			// TODO: Integrate this flag into collision-events in some way.
			// 
			// Similar to `hit_entity_is_child`, we update this value later,
			// but until then, we assume false to be correct.
			bool hit_entity_is_parent = false;

			if (hit_resolution) // <-- Currently a requirement for enumeration; may change later.
			{
				bool hit_child_enumeration_required  = ((!hit_resolution->can_be_influenced_by_children) || (!hit_resolution->can_influence_children)); // || ...

				if (hit_child_enumeration_required)
				{
					// Since the `hit_entity` is stating that children can't influence it,
					// we'll need to determine if we're one of its children:
					if (auto* hit_relationship = registry.try_get<Relationship>(hit_entity))
					{
						// Enumerate `hit_entity`'s children, comparing against ourself (`entity`):
						hit_relationship->enumerate_children(registry, [entity, &hit_entity_is_parent](Entity child, const Relationship& child_relationship, Entity next_child) -> bool
						{
							if (child == entity)
							{
								// We are one of `hit_entity`'s children, stop enumeration:
								hit_entity_is_parent = true;

								// Short-circuit enumeration.
								return false;
							}

							// Continue enumeration.
							return true;
						}, true);
					}
				}
			}

			// Determine if we need to look through our relationship tree:
			bool child_enumeration_required = ((!kinematic_resolution.can_influence_children) || (!kinematic_resolution.can_be_influenced_by_children));

			if (child_enumeration_required)
			{
				if (auto* relationship = registry.try_get<Relationship>(entity))
				{
					// Enumerate this entity's children, comparing against `hit_entity`:
					relationship->enumerate_children(registry, [hit_entity, &hit_entity_is_child, &kinematic_resolution, &apply_influence, &apply_correction](Entity child, const Relationship& child_relationship, Entity next_child) -> bool
					{
						if (child == hit_entity)
						{
							if (!kinematic_resolution.can_influence_children)
							{
								// The entity we hit is a child and we're not allowed to influence it.
								apply_influence = false;
							}

							if (!kinematic_resolution.can_be_influenced_by_children)
							{
								apply_correction = false;
							}

							hit_entity_is_child = true;

							// Short-circuit enumeration.
							return false;
						}

						// Continue enumeration.
						return true;
					}, true);
				}
			}

			auto handle_influence_on_hit_entity = [&]()
			{
				// Check if influences are enabled:
				if (!apply_influence)
				{
					return;
				}

				// Check if we're allowed to influence an object:
				if (!kinematic_resolution.is_influencer)
				{
					return;
				}

				// Check if we're supposed to resolve this hit/influence:
				if (!hit_resolution.has_value())
				{
					// A `hit_resolution` instance is required to influence an object.
					return;
				}

				// Ensure the object we're influencing allows it.
				if (!hit_resolution->accepts_influence)
				{
					return;
				}

				// Check if we're a child of `entity`:
				if (hit_entity_is_parent)
				{
					// Check if `hit_entity` forbids influences from children:
					if (!hit_resolution->can_be_influenced_by_children)
					{
						// `hit_entity` reported that we can't influence it; return immediately.
						return;
					}
				}

				// The strength (%) of movement translated from `entity` to `hit_entity`.
				float influence_strength = 1.0f;

				const auto hit_mass = hit_collision->get_mass();

				// Objects with a mass of exactly 0.0 ('Infinite mass') are influenced the full amount.
				// For objects with non-zero mass, use the ratio between the 'moving' object and the 'hit' object:
				if (hit_mass > 0.0f)
				{
					const auto mass = collision.get_mass();

					//const auto mass_ratio = std::min(((mass - hit_mass) / mass), 1.0f);
					const auto mass_ratio = std::min((mass / hit_mass), 1.0f);

					influence_strength = mass_ratio;
				}
				else
				{
					/*
						If we're moving the `hit_entity` forward the full distance,
						we don't need to correct the path of `entity`.
								
						Likewise, skippping the adjustment phase will
						forego the `OnKinematicAdjustment` event.
					*/
					apply_correction = false;
				}

				// Determine how far `entity` intended to move:
				const auto intended_movement_distance = glm::length(impact_velocity);
				
				// The influence applied to the `hit_entity` is the length of the intended movement, multiplied by
				// the reversed direction of the surface normal (now forward, instead of backward)
				// that intersected the originating entity's path.
				influence = ((intended_movement_distance * influence_strength) * -hit_normal);

				// Update the transform of `hit_entity` 
				auto hit_tform = world.get_transform(hit_entity);
				
				auto hit_old_position = hit_tform.get_position();
				auto hit_new_position = (hit_old_position + influence);

				hit_tform.set_position(hit_new_position);
						
				// Re-validation is not needed at this time.
				// (Allows for further resolution on the next update)
				//hit_tform.validate_collision_shallow();

				// Notify listeners that `hit_entity` was moved (influenced) kinematically.
				world.queue_event
				(
					OnKinematicInfluence
					{
						.influencer = entity,

						.target =
						{
							.entity = hit_entity,

							.old_position = hit_old_position,
							.new_position = hit_new_position,

							.influence_applied = influence
						},
								
						.contact =
						{
							.point  = hit_point_in_world,
							.normal = hit_normal
						}
					}
				);
			};

			auto adjust_entity_position = [&]()
			{
				if (!apply_correction)
				{
					return;
				}

				// Check if `hit_entity` is our parent:
				if (hit_entity_is_parent)
				{
					if (hit_resolution)
					{
						// Check if this adjustment falls within the influencing rules of our parent:
						if (!hit_resolution->can_influence_children)
						{
							return;
						}
					}
				}

				const auto& hit_adjustment = penetration;

				auto object_half_dimensions = kinematic_resolution.get_half_size_vector(collision);

				auto edge_offset = (hit_normal * object_half_dimensions);
				auto correction  = (edge_offset - hit_adjustment);

				auto adjusted_position = (new_position + correction + influence);
				
				transform.set_position(adjusted_position);

				// TODO: May cause side effect of collision-update happening again. (Need to look into this more)
				// Re-validation not needed at this time.
				//transform.validate_collision_shallow();

				const auto& expected_position = new_position;

				world.queue_event<OnKinematicAdjustment>
				(
					entity, hit_entity,
					old_position, adjusted_position, expected_position
				);
			};

			handle_influence_on_hit_entity();
			adjust_entity_position();

			// NOTE: Faster than always handling another `Transform` object,
			// or somehow forwarding from the `handle_influence_on_hit_entity` step.
			auto hit_old_position = math::to_vector(hit_object->getWorldTransform().getOrigin());

			// Notify listeners that this `entity` contacted `hit_entity`'s surface:
			world.event // world.queue_event
			(
				OnSurfaceContact
				{
					// General information.
					.collision =
					{
						.a = entity,
						.b = hit_entity,

						.a_position = old_position,
						.b_position = hit_old_position,

						.position = hit_point_in_world,
						.normal   = hit_normal,

						.penetration = glm::length(penetration),

						.native =
						{
							.a_object = &collision_obj,
							.b_object = hit_object
						},

						.contact_type = ContactType::Surface
					},
					
					.impact_velocity = impact_velocity
				}
			);
		};

		// TODO: Look into whether we should replace this switch-statement with the `cast_to` member-function.
		switch (kinematic_resolution.cast_method)
		{
			case CollisionCastMethod::ConvexCast:
			{
				auto result = convex_cast_to(*this, collision, transform.get_matrix());

				if (result)
				{
					kinematic_resolve(result);
				}

				break;
			}

			// TODO: Look into whether we want to implement this.
			//case CollisionCastMethod::ConvexKinematicCast:
				// INSERT USE OF `btKinematicClosestNotMeConvexResultCallback` here.
				//break;

			case CollisionCastMethod::RayCast:
			{
				auto result = ray_cast_to(*this, collision, transform.get_position());

				if (result)
				{
					kinematic_resolve(result);
				}

				break;
			}
			
			//case CollisionCastMethod::None:
			default:
				break;
		}
	}

	std::optional<CollisionCastResult> PhysicsSystem::cast_to
	(
		const RayCastSelf& obj,
		const math::Vector& destination,

		std::optional<CollisionGroup> filter_group,
		std::optional<CollisionGroup> filter_mask
	)
	{
		CollisionCastMethod cast_method = CollisionCastMethod::None;

		std::optional<CollisionCastResult> result = std::nullopt;

		util::visit
		(
			obj,

			[&](const btCollisionObject& c_obj)
			{
				const auto* shape = c_obj.getCollisionShape();
				const btConvexShape* convex_shape = nullptr;

				bool is_convex = shape->isConvex();

				if (is_convex)
				{
					// NOTE: Safer option, but significantly slower.
					//convex_shape = dynamic_cast<const btConvexShape*>(shape);
					//is_convex = (convex_shape != nullptr);

					// NOTE: This option is faster, but it takes Bullet's `getShapeType` at face-value. (Possibly unsafe)
					convex_shape = reinterpret_cast<const btConvexShape*>(shape);
				}

				if (is_convex)
				{
					result = convex_cast_to
					(
						*this,
						CollisionObjectAndConvexShape{ c_obj, *convex_shape },
						destination, filter_group, filter_mask
					);
				}
				else
				{
					result = ray_cast_to(*this, c_obj, destination, filter_group, filter_mask);
				}
			},

			[&](const CollisionComponent& collision)
			{
				result = cast_to(collision, destination, filter_group, filter_mask);
			},

			[&](Entity entity)
			{
				auto& registry = world.get_registry();
				auto* collision = registry.try_get<CollisionComponent>(entity);

				if (collision)
				{
					result = cast_to(*collision, destination, filter_group, filter_mask);
				}
			},

			[](const std::monostate&) {}
		);

		return result;
	}

	std::optional<CollisionCastResult> PhysicsSystem::cast_to
	(
		const CollisionComponent& collision,
		const math::Vector& destination,

		std::optional<CollisionGroup> filter_group,
		std::optional<CollisionGroup> filter_mask,

		bool check_kinematic_resolution
	)
	{
		bool is_convex = collision.is_convex();

		if (is_convex && check_kinematic_resolution)
		{
			auto kinematic_resolution_opt = collision.get_kinematic_resolution();

			if (kinematic_resolution_opt.has_value())
			{
				is_convex = (kinematic_resolution_opt->cast_method == CollisionCastMethod::ConvexCast);
			}
		}

		if (is_convex)
		{
			return convex_cast_to(*this, collision, destination, filter_group, filter_mask);
		}

		return ray_cast_to(*this, collision, destination, filter_group, filter_mask);
	}

	void PhysicsSystem::on_create_collider(Registry& registry, Entity entity)
	{
		//auto& world = new_col.world;

		auto transform = world.get_transform(entity);
		auto& component = registry.get<CollisionComponent>(entity);

		bool object_found = component.on_collision_object
		(
			[this, &component, &transform](const auto& collision_obj)
			{
				auto c_group = static_cast<int>(component.get_group());
				auto c_mask = static_cast<int>(component.get_full_mask());

				using obj_type = std::decay_t<decltype(collision_obj)>;

				if constexpr (std::is_same_v<obj_type, CollisionComponent::RigidBody>)
				{
					this->collision_world->addRigidBody(collision_obj.get(), c_group, c_mask);
				}
				else
				{
					this->collision_world->addCollisionObject(collision_obj.get(), c_group, c_mask);
				}

				update_collision_object(*collision_obj, transform);
			}
		);
	}

	void PhysicsSystem::on_destroy_collider(Registry& registry, Entity entity)
	{
		auto* component = registry.try_get<CollisionComponent>(entity);

		if (!component)
		{
			return;
		}

		auto* collision_obj = component->get_collision_object();

		if (!collision_obj)
		{
			return;
		}

		/*
			Bullet handles checks for derived collision-object types and handles them
			appropriately, so we only need to call `removeCollisionObject` here.
			
			e.g. if the object is actually of type `btRigidBody`, Bullet
			will call `removeRigidBody` automatically.
		*/
		collision_world->removeCollisionObject(collision_obj);
	}

	void PhysicsSystem::update_collision_object(CollisionComponent& col, Transform& transform) // World& world, Entity entity
	{
		// TODO: Determine if any addition work needs to be done in this step for other collision-object types. (e.g. `btRigidBody`)

		auto* collision_obj = col.get_collision_object();

		if (collision_obj)
		{
			update_collision_object(*collision_obj, transform);
		}
	}

	void PhysicsSystem::update_collision_object(btCollisionObject& obj, Transform& transform)
	{
		update_collision_object(obj, transform.get_matrix());
	}

	void PhysicsSystem::update_collision_object(btCollisionObject& obj, const math::Matrix& m)
	{
		update_collision_object_transform(obj, m);
	}

	// Internal shorthand for `world.get_gravity()`.
	math::Vector PhysicsSystem::get_gravity() const
	{
		return world.get_gravity();
	}

	// Internal shorthand for `world.down()`.
	math::Vector PhysicsSystem::down() const
	{
		return world.down();
	}

	void PhysicsSystem::set_physics_gravity(const math::Vector& gravity)
	{
		collision_world->setGravity(math::to_bullet_vector(gravity));
	}

	math::Vector PhysicsSystem::get_physics_gravity() const
	{
		return math::to_vector(collision_world->getGravity());
	}

	template <typename CollisionEventType>
	void PhysicsSystem::forward_collision_event(const CollisionEventType& event_obj)
	{
		// Automatically forward the `collision` field to `OnCollision` listeners.
		world.event<OnCollision>(event_obj.collision);
	}

	void PhysicsSystem::on_surface_contact(const OnSurfaceContact& surface)
	{
		forward_collision_event(surface);
	}

	void PhysicsSystem::on_intersection(const OnIntersection& intersection)
	{
		forward_collision_event(intersection);
	}

	// Internal routine, used for `ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL`.
	void PhysicsSystem::retrieve_bullet_transforms()
	{
		#if defined(ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL) && (ENGINE_COLLISION_MOTION_STATE_ALTERNATIVE_IMPL == 1)
			auto& registry = world.get_registry();

			// Retrieve newly reported transform states from Bullet:
			auto view = registry.view<CollisionComponent>();

			view.each([this](auto entity, CollisionComponent& collision)
			{
				auto* motion_state = collision.get_motion_state();

				if (!motion_state)
				{
					return;
				}

				auto new_matrix = motion_state->retrieve_from_bullet();

				if (new_matrix.has_value())
				{
					auto transform = world.get_transform(entity);

					transform.set_matrix(*new_matrix);
				}
			});
		#endif
	}
}