//#include "collision.hpp"
#include "collision_component.hpp"

#include <engine/world/world.hpp>

#include <math/bullet.hpp>

#include <bullet/btBulletCollisionCommon.h>
//#include <bullet/btBulletDynamicsCommon.h>
//#include <bullet/BulletCollision/CollisionDispatch/btCollisionObject.h>

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

	CollisionComponent::~CollisionComponent() {}

	void CollisionComponent::set_shape(const Shape& shape)
	{
		collision->setCollisionShape(shape.get());

		this->shape = shape;
	}

	void CollisionComponent::set_shape(const ConcaveShape& shape) // Same as 'Shape' implementation, but with specific type.
	{
		collision->setCollisionShape(shape.get());

		this->shape = shape;
	}

	void CollisionComponent::set_shape(const ConvexShape& shape) // Same as 'Shape' implementation, but with specific type.
	{
		collision->setCollisionShape(shape.get());

		this->shape = shape;
	}

	btCollisionObject* CollisionComponent::get_collision_object()
	{
		return collision.get();
	}

	const btCollisionObject* CollisionComponent::get_collision_object() const
	{
		return collision.get();
	}

	void CollisionComponent::activate(bool force)
	{
		collision->activate(force);
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

	std::unique_ptr<btCollisionObject> CollisionComponent::make_collision_object(const CollisionConfig& config, float mass)
	{
		using native_flags = btCollisionObject::CollisionFlags;
		using enum native_flags;

		auto obj = std::make_unique<btCollisionObject>();

		// TODO: Look into `btRigidBodyConstructionInfo` version of `btRigidBody`'s constructor.
		//btRigidBody(...)
		//auto obj = std::make_unique<btRigidBody>(mass);

		/*
		// Disabled for now:
		std::underlying_type_t<native_flags> flags = 0;

		if ((config.group & CollisionGroup::StaticGeometry))
		{
			// Bullet does not allow a collision object to be both static and dynamic.
			assert(!(config.group & CollisionGroup::DynamicGeometry));

			flags |= CF_STATIC_OBJECT;
		}

		// No other flags are used at the moment.

		obj->setCollisionFlags(static_cast<native_flags>(flags));
		*/

		// For safety, we always set the associated `Entity` to `null` by default.
		set_entity_for_collision_object(*obj, null);

		return obj;
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