//#include "collision.hpp"
#include "collision_component.hpp"
#include "collision_motion_state.hpp"

#include <engine/world/world.hpp>

#include <math/bullet.hpp>

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
//#include <bullet/BulletCollision/CollisionDispatch/btCollisionObject.h>

#include <utility>

namespace engine
{
	// Only accessible in this file.
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

	CollisionComponent::CollisionComponent(const CollisionConfig& config, float mass, std::unique_ptr<CollisionMotionState>&& motion_state) :
		CollisionConfig(config),
		mass(mass),
		motion_state(std::move(motion_state))
	{}

	template <typename ShapeRefType>
	void CollisionComponent::set_shape_impl(ShapeRefType&& shape)
	{
		if (!shape)
		{
			return;
		}

		if (collision)
		{
			collision->setCollisionShape(shape.get());
		}

		this->shape = shape;
	}
	
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
		}
		else
		{
			if ((group & CollisionGroup::DynamicGeometry))
			{
				// Note: `CF_DYNAMIC_OBJECT` is typically `0`, so this is effectively a no-op.
				// (Added here for consistency; Bullet could change its bitfields, etc.)
				flags |= CF_DYNAMIC_OBJECT;
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

		return (flags & CF_KINEMATIC_OBJECT);
	}

	// Acceptable inputs to give to `set_shape_impl`:
	void CollisionComponent::set_shape(const Shape& shape)        { set_shape_impl(std::forward<decltype(shape)>(shape)); }
	void CollisionComponent::set_shape(const ConcaveShape& shape) { set_shape_impl(std::forward<decltype(shape)>(shape)); }
	void CollisionComponent::set_shape(const ConvexShape& shape)  { set_shape_impl(std::forward<decltype(shape)>(shape)); }

	void CollisionComponent::set_collision_object(std::unique_ptr<btCollisionObject>&& collision)
	{
		if (collision)
		{
			// For safety, we always set the associated `Entity` to `null` by default.
			set_entity_for_collision_object(*collision, null);

			this->collision = std::move(collision);
		}
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

		auto rigid_body = std::make_unique<btRigidBody>(mass, this->motion_state.get(), collision_shape); // , localInertia=btVector3(0, 0, 0)

		auto obj_info = apply_collision_flags(*rigid_body, config, true, allow_kinematics);

		// Re-encapsulate our handle to `rigid_body` as its parent type,
		// `btCollisionObject`, then assign the collision-object instance accordingly.
		set_collision_object(std::unique_ptr<btCollisionObject>(rigid_body.release()));

		return obj_info;
	}

	btCollisionObject* CollisionComponent::get_collision_object()
	{
		return collision.get();
	}

	const btCollisionObject* CollisionComponent::get_collision_object() const
	{
		return collision.get();
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
		if (collision)
		{
			collision->activate(force);

			return is_active();
		}

		return false;
	}

	bool CollisionComponent::deactivate(bool force)
	{
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
		if (collision)
		{
			return collision->isActive();
		}

		return false;
	}

	bool CollisionComponent::is_kinematic() const
	{
		using enum btCollisionObject::CollisionFlags;

		if (collision)
		{
			return (collision->getCollisionFlags() & CF_KINEMATIC_OBJECT);
		}

		return false;
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

	CollisionComponent::ConvexShape CollisionComponent::get_convex_shape() const
	{
		if (auto out = std::get_if<ConvexShape>(&shape))
		{
			return *out;
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

		// TODO: Hook into entt's Entity creation, destruction and component-assignment events to better handle this.
		// At the moment, we just trigger the event immediately:
		world.event<OnComponentAdd<CollisionComponent>>(entity); // Entity destruction currently handles this event manually as well. (needs to be changed)

		return entity;
	}
}