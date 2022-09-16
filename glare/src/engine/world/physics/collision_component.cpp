//#include "collision.hpp"
#include "collision_component.hpp"
#include "collision_motion_state.hpp"

#include <engine/world/world.hpp>

#include <math/bullet.hpp>

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

//#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>

namespace engine
{
	static void set_entity_for_collision_object(btCollisionObject& c_obj, Entity entity)
	{
		static_assert(sizeof(int) >= sizeof(entt::id_type));
		c_obj.setUserIndex(static_cast<int>(entity));
	}

	// Unlike `set_entity_for_collision_object`, this function is part of the public API.
	Entity get_entity_from_collision_object(const btCollisionObject& c_obj)
	{
		auto index_raw = c_obj.getUserIndex();

		/*
		if (index_raw == 0)
		{
			return null;
		}
		*/

		return static_cast<Entity>(index_raw);
	}

	void update_collision_object_transform(btCollisionObject& obj, const math::Matrix& m)
	{
		obj.setWorldTransform(math::to_bullet_matrix(m));
	}

	void CollisionComponent::set_entity_for_collision_object(btCollisionObject& c_obj, Entity entity)
	{
		engine::set_entity_for_collision_object(c_obj, entity);
	}

	// Unlike `set_entity_for_collision_object`, this function is part of the public API.
	Entity CollisionComponent::get_entity_from_collision_object(const btCollisionObject& c_obj)
	{
		return engine::get_entity_from_collision_object(c_obj);
	}

	CollisionComponent::CollisionComponent(const CollisionConfig& config, std::unique_ptr<CollisionMotionState>&& motion_state) :
		CollisionConfig(config),
		motion_state(std::move(motion_state)),
		collision_object(std::monostate())
	{}
	
	bool CollisionComponent::apply_collision_flags(btCollisionObject& c_obj, const CollisionConfig& config, bool keep_existing_flags, bool allow_kinematic)
	{
		using native_flags = btCollisionObject::CollisionFlags;
		using bitfield_t = std::underlying_type_t<native_flags>;
		using enum native_flags;

		bitfield_t flags = ((keep_existing_flags) ? flags = c_obj.getCollisionFlags() : 0);

		auto group = config.group;

		if ((group & CollisionGroup::StaticGeometry))
		{
			// Bullet does not allow a collision object to be both static and dynamic.
			assert(!(group & CollisionGroup::DynamicGeometry));

			flags |= CF_STATIC_OBJECT;

			// Debugging related:
			//flags |= CF_KINEMATIC_OBJECT;
		}
		else
		{
			if ((group & CollisionGroup::DynamicGeometry))
			{
				// Note: `CF_DYNAMIC_OBJECT` is typically `0`, so this is effectively a no-op.
				// (Added here for consistency; Bullet could change its bitfields, etc.)
				flags |= CF_DYNAMIC_OBJECT;

				// Ensure the static object flag isn't set.
				flags &= ~CF_STATIC_OBJECT;
			}


			// TODO:
			// This may be something we revisit at a later date.
			// For now, take the group-based lookup at face value.
			bool is_kinematic = collision_group_is_kinematic(group);

			if (is_kinematic)
			{
				flags |= CF_KINEMATIC_OBJECT;
			}
		}

		if (!allow_kinematic)
		{
			// Force-disable kinematic status.
			flags &= ~CF_KINEMATIC_OBJECT;
		}

		// TODO:
		// No other flags are used at the moment.
		// We may need to revisit this in the future.
		/*
		* Possible flags:
		CF_DYNAMIC_OBJECT = 0,
		CF_STATIC_OBJECT = 1,
		CF_KINEMATIC_OBJECT = 2,
		CF_NO_CONTACT_RESPONSE = 4,
		CF_CUSTOM_MATERIAL_CALLBACK = 8,  //this allows per-triangle material (friction/restitution)
		CF_CHARACTER_OBJECT = 16,
		CF_DISABLE_VISUALIZE_OBJECT = 32,          //disable debug drawing
		CF_DISABLE_SPU_COLLISION_PROCESSING = 64,  //disable parallel/SPU processing
		CF_HAS_CONTACT_STIFFNESS_DAMPING = 128,
		CF_HAS_CUSTOM_DEBUG_RENDERING_COLOR = 256,
		CF_HAS_FRICTION_ANCHOR = 512,
		CF_HAS_COLLISION_SOUND_TRIGGER = 1024
		*/

		c_obj.setCollisionFlags(static_cast<native_flags>(flags));

		// Debugging related:
		if (flags & CF_KINEMATIC_OBJECT)
		{
			// Disable deactivation of kinematic objects.
			c_obj.setActivationState(DISABLE_DEACTIVATION);
		}

		return (flags & CF_KINEMATIC_OBJECT);
	}

	bool CollisionComponent::collision_object_in_monostate() const
	{
		return (collision_object.index() == null_collision_object_index);
	}

	const CollisionComponent::collision_object_variant_t& CollisionComponent::get_collision_object_variant() const
	{
		return collision_object;
	}

	void CollisionComponent::build_basic_collision_object(const CollisionConfig& config) // , float mass
	{
		auto collision = std::make_unique<btCollisionObject>();

		apply_collision_flags(*collision, config, true, false);

		set_collision_object(std::move(collision));
	}

	bool CollisionComponent::build_rigid_body(const CollisionConfig& config, btCollisionShape* collision_shape, float mass, bool allow_kinematics)
	{
		using native_flags = btCollisionObject::CollisionFlags;
		using enum native_flags;

		// `set_shape` must be called first.
		assert((shape.index() != std::variant_npos));

		// `collision_shape` cannot be nullptr. (we take a pointer to indicate address permanency; see declaration)
		assert(collision_shape);

		auto details = btRigidBody::btRigidBodyConstructionInfo(mass, this->motion_state.get(), collision_shape); // , localInertia=btVector3(0, 0, 0)

		details.m_restitution = 0.8f;
		details.m_friction = 0.5f;

		auto rigid_body = std::make_unique<btRigidBody>(details);

		auto obj_info = apply_collision_flags(*rigid_body, config, true, allow_kinematics);

		set_collision_object(rigid_body);

		return obj_info;
	}

	KinematicResolutionConfig CollisionComponent::default_kinematic_resolution_method() const // std::optional<KinematicResolutionConfig>
	{
		// Collision-casting on transformation-change is only
		// possible with kinematic objects, currently.
		if (!is_kinematic())
		{
			return {};
		}

		// Convex shapes get to utilize Bullet's convex-sweep functionality.
		if (peek_convex_shape())
		{
			////return { CollisionCastMethod::ConvexCast, KinematicResolutionConfig::AABBType {}, true, true }; // CollisionCastMethod::ConvexKinematicCast
			return { CollisionCastMethod::ConvexCast, KinematicResolutionConfig::InnerSphereType {}, true, true, true };
		}

		// Everything else gets a simple ray-cast approach.
		////return { CollisionCastMethod::RayCast, KinematicResolutionConfig::AABBType {}, true, true };
		return { CollisionCastMethod::RayCast, KinematicResolutionConfig::InnerSphereType {}, true, true, true };
	}

	btCollisionObject* CollisionComponent::get_collision_object()
	{
		return const_cast<btCollisionObject*>(reinterpret_cast<const CollisionComponent*>(this)->get_collision_object());
	}

	const btCollisionObject* CollisionComponent::get_collision_object() const
	{
		btCollisionObject* result = nullptr;

		on_collision_object
		(
			[&result](const auto& obj)
			{
				result = obj.get();
			}
		);

		return result;
	}

	btRigidBody* CollisionComponent::get_rigid_body()
	{
		return const_cast<btRigidBody*>(reinterpret_cast<const CollisionComponent*>(this)->get_rigid_body());
	}

	const btRigidBody* CollisionComponent::get_rigid_body() const
	{
		const auto* rigid_body = std::get_if<RigidBody>(&collision_object);

		if (rigid_body)
		{
			return rigid_body->get();
		}

		return nullptr;
	}

	CollisionMotionState* CollisionComponent::get_motion_state()
	{
		return motion_state.get();
	}
	
	const CollisionMotionState* CollisionComponent::get_motion_state() const
	{
		return motion_state.get();
	}

	bool CollisionComponent::activate(bool force)
	{
		auto* collision = get_collision_object();

		if (collision)
		{
			collision->activate(force);

			return is_active();
		}

		return false;
	}

	bool CollisionComponent::deactivate(bool force)
	{
		auto* collision = get_collision_object();

		if (collision)
		{
			if (force)
			{
				collision->forceActivationState(DISABLE_SIMULATION);
			}
			else
			{
				collision->setActivationState(DISABLE_SIMULATION);
			}

			return !is_active();
		}

		return true;
	}

	bool CollisionComponent::is_active() const
	{
		auto* collision = get_collision_object();

		if (collision)
		{
			return collision->isActive();
		}

		return false;
	}

	bool CollisionComponent::is_kinematic() const
	{
		using enum btCollisionObject::CollisionFlags;

		auto* collision = get_collision_object();

		if (collision)
		{
			return (collision->getCollisionFlags() & CF_KINEMATIC_OBJECT);
		}

		return false;
	}

	bool CollisionComponent::is_dynamic() const
	{
		using enum btCollisionObject::CollisionFlags;

		auto* collision = get_collision_object();

		if (collision)
		{
			return (collision->getCollisionFlags() & CF_DYNAMIC_OBJECT); // && !is_static()
		}

		return false;
	}

	bool CollisionComponent::is_static() const
	{
		//return !is_dynamic();

		using enum btCollisionObject::CollisionFlags;

		auto* collision = get_collision_object();

		if (collision)
		{
			return (collision->getCollisionFlags() & CF_STATIC_OBJECT); // && !is_dynamic();
		}

		return false;
	}

	bool CollisionComponent::has_collision_object() const
	{
		if (collision_object_in_monostate())
		{
			return false;
		}

		return (get_collision_object());
	}

	bool CollisionComponent::has_motion_state() const
	{
		return (get_motion_state());
	}

	float CollisionComponent::get_mass() const
	{
		auto* rigid_body = get_rigid_body();

		if (rigid_body)
		{
			return rigid_body->getMass();
		}

		return 0.0f;
	}

	void CollisionComponent::set_mass(float mass)
	{
		auto* rigid_body = get_rigid_body();

		if (rigid_body)
		{
			rigid_body->setMassProps(mass, rigid_body->getLocalInertia());
		}
	}

	bool CollisionComponent::has_kinematic_cast_method() const
	{
		if (kinematic_resolution.has_value())
		{
			return (kinematic_resolution->cast_method != CollisionCastMethod::None);
		}

		return false;
	}

	std::tuple<math::Vector, math::Vector> CollisionComponent::get_world_aabb() const
	{
		btVector3 aabb_min, aabb_max;

		//btVector3 aabb_min = btVector3(0.0f, 0.0f, 0.0f);
		//btVector3 aabb_max = btVector3(0.0f, 0.0f, 0.0f);

		const auto* rigid_body = get_rigid_body();

		if (rigid_body)
		{
			rigid_body->getAabb(aabb_min, aabb_max);
		}
		else
		{
			const auto* c_obj = get_collision_object();

			if (!c_obj)
			{
				return {};
			}

			c_obj->getCollisionShape()->getAabb(c_obj->getWorldTransform(), aabb_min, aabb_max);
		}

		return { math::to_vector(aabb_min), math::to_vector(aabb_max) };
	}

	std::tuple<math::Vector, float> CollisionComponent::get_world_bounding_sphere() const
	{
		// Values assigned for safety:
		btVector3 sphere_center = {};
		float sphere_radius = 0.0f;

		auto* shape = peek_shape();
		const auto* c_obj = get_collision_object();

		if (!shape)
		{
			return {};
		}

		if (!c_obj)
		{
			return {};
		}

		shape->getBoundingSphere(sphere_center, sphere_radius);

		return { math::to_vector((c_obj->getWorldTransform() * sphere_center)), sphere_radius };
	}

	float CollisionComponent::get_aabb_size() const
	{
		auto [aabb_min, aabb_max] = get_world_aabb();

		// Compute the length of the delta from the 'min' and 'max' vectors. (Pythagorean theorem)
		//return glm::length(math::to_vector(aabb_max - aabb_min));
		return glm::length(math::to_vector(aabb_max - aabb_min));
	}

	math::Vector CollisionComponent::get_aabb_lengths() const
	{
		auto [aabb_min, aabb_max] = get_world_aabb();
		return (aabb_max - aabb_min);
	}

	float CollisionComponent::get_bounding_radius() const
	{
		// Unified implementation (slower):
		//auto [sphere_center, sphere_radius] = get_world_bounding_sphere();
		//return sphere_radius;

		// Optimized implementation:
		auto* shape = peek_shape();

		if (!shape)
		{
			return 0.0f;
		}

		btVector3 _sphere_center;
		float sphere_radius = 0.0f; // <-- Value assigned for safety.

		shape->getBoundingSphere(_sphere_center, sphere_radius);

		return sphere_radius;
	}

	float CollisionComponent::get_inner_radius() const
	{
		//return (get_inner_diameter() * 0.5f);

		auto delta = (get_aabb_lengths() * 0.5f);

		return ((delta.x + delta.y + delta.z) / 3.0f);
	}

	float CollisionComponent::get_inner_diameter() const
	{
		auto delta = get_aabb_lengths();

		return ((delta.x + delta.y + delta.z) / 3.0f);
	}

	CollisionCastMethod CollisionComponent::get_kinematic_cast_method() const
	{
		if (kinematic_resolution.has_value())
		{
			return kinematic_resolution->cast_method;
		}

		return CollisionCastMethod::None;
	}

	void CollisionComponent::set_kinematic_resolution(const KinematicResolutionConfig& resolution)
	{
		kinematic_resolution = resolution;
	}

	const std::optional<KinematicResolutionConfig>& CollisionComponent::get_kinematic_resolution() const
	{
		return kinematic_resolution;
	}

	void CollisionComponent::update_transform(const math::Matrix& tform)
	{
		auto* collision = get_collision_object();

		if (!collision)
		{
			return;
		}

		update_collision_object_transform(*collision, tform);
	}

	CollisionComponent::Shape CollisionComponent::get_shape() const
	{
		Shape out;

		std::visit
		(
			[&](auto&& value)
			{
				out = std::static_pointer_cast<RawShape>(value);
			},

			this->shape
		);

		return out;
	}

	CollisionComponent::RawShape* CollisionComponent::peek_shape() const
	{
		RawShape* out = nullptr;

		std::visit
		(
			[&](auto&& value)
			{
				out = std::static_pointer_cast<RawShape>(value).get();
			},

			this->shape
		);

		return out;
	}

	CollisionComponent::ConvexShape CollisionComponent::get_convex_shape() const
	{
		if (auto out = std::get_if<ConvexShape>(&shape))
		{
			return *out;
		}

		return {};
	}

	CollisionComponent::ConvexShapeRaw* CollisionComponent::peek_convex_shape() const
	{
		auto out = std::get_if<ConvexShape>(&shape);

		if (out)
		{
			return (*out).get();
		}

		return {};
	}

	CollisionComponent::ConcaveShape CollisionComponent::get_concave_shape() const
	{
		if (auto out = std::get_if<ConcaveShape>(&shape))
		{
			return *out;
		}

		return {};
	}

	CollisionComponent::ConcaveShapeRaw* CollisionComponent::peek_concave_shape() const
	{
		auto out = std::get_if<ConcaveShape>(&shape);

		if (out)
		{
			return (*out).get();
		}

		return {};
	}
	
	Entity attach_collision_impl(World& world, Entity entity, CollisionComponent&& col)
	{
		auto& registry = world.get_registry();

		auto& component = registry.emplace<CollisionComponent>(entity, std::move(col));

		auto* c_obj = component.get_collision_object();

		assert(c_obj);

		set_entity_for_collision_object(*c_obj, entity);

		// Note: Unsafe due to entt's ability to move/reallocate this component. (Used for debugging purposes only)
		#ifndef NDEBUG
			c_obj->setUserPointer(&component);
		#endif

		// NOTE: `PhysicsSystem` hooks into entt's component events in order to add/remove `c_obj` from the collision world.

		return entity;
	}
}