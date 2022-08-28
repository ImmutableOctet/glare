#include "collision.hpp"
#include "world/world.hpp"
#include "types.hpp"

#include <math/bullet.hpp>

//#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletCollisionCommon.h>
//#include <bullet/btCollisionObject.h>
//#include <bullet/BulletCollision/btBulletCollisionCommon.h>

#include <utility>
#include <type_traits>

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

	CollisionGroup CollisionConfig::resolve_collision_group(EntityType type)
	{
		switch (type)
		{
			case EntityType::Geometry: return CollisionGroup::StaticGeometry;
			
			case EntityType::Platform: return CollisionGroup::DynamicGeometry;
			case EntityType::Crusher: return CollisionGroup::DynamicGeometry;
			
			case EntityType::Bone: return CollisionGroup::Bone;

			case EntityType::Object: return CollisionGroup::Object;
			case EntityType::Camera: return CollisionGroup::Object;
			
			case EntityType::Player: return CollisionGroup::Actor;
			case EntityType::Enemy: return CollisionGroup::Actor;
			case EntityType::FriendlyActor: return CollisionGroup::Actor;

			case EntityType::Collectable: return CollisionGroup::Object;
			
			case EntityType::Particle: return CollisionGroup::Particle; // CollisionGroup::All;
			
			case EntityType::WaterZone: return CollisionGroup::Zone;
			case EntityType::KillZone: return CollisionGroup::Zone;
			case EntityType::DamageZone: return CollisionGroup::Zone;
			case EntityType::EventTrigger: return CollisionGroup::Zone;

			case EntityType::Projectile: return CollisionGroup::Projectile;

			//case EntityType::Light: return CollisionGroup::StaticGeometry;
			//case EntityType::Pivot: return CollisionGroup::None;
			//case EntityType::Regulator: return CollisionGroup::None;
			//case EntityType::Generator: return CollisionGroup::None;
		}

		return CollisionGroup::None;
	}

	CollisionGroup CollisionConfig::resolve_solid_mask(EntityType type)
	{
		switch (type)
		{
			case EntityType::Geometry: return CollisionGroup::GeometrySolids;
			case EntityType::Platform: return CollisionGroup::GeometrySolids;
			case EntityType::Crusher: return CollisionGroup::GeometrySolids; // (CollisionGroup::Actor | CollisionGroup::Projectile);
			//case EntityType::Light: return CollisionGroup::None;
			//case EntityType::Pivot: return CollisionGroup::None;
			case EntityType::Bone: return CollisionGroup::BoneSolids;
			case EntityType::Object: return CollisionGroup::ObjectSolids;
			case EntityType::Player: return CollisionGroup::ActorSolids;
			case EntityType::Camera: return CollisionGroup::ObjectSolids; // CollisionGroup::None;
			case EntityType::Enemy: return CollisionGroup::ActorSolids;
			case EntityType::FriendlyActor: return CollisionGroup::ActorSolids;
			case EntityType::Collectable: return CollisionGroup::AllGeometry;
			case EntityType::Particle: return CollisionGroup::AllGeometry; // CollisionGroup::ObjectSolids;
			case EntityType::Projectile: return CollisionGroup::ProjectileSolids; // CollisionGroup::None
			//case EntityType::EventTrigger: return CollisionGroup::None;

			case EntityType::WaterZone: return CollisionGroup::None;
			case EntityType::KillZone: return CollisionGroup::None;
			case EntityType::DamageZone: return CollisionGroup::None;

			case EntityType::Generator: return CollisionGroup::AllGeometry; // CollisionGroup::None
		}

		return CollisionGroup::None;
	}

	CollisionGroup CollisionConfig::resolve_interaction_mask(EntityType type)
	{
		switch (type)
		{
			case EntityType::Geometry: return CollisionGroup::None;
			case EntityType::Platform: return CollisionGroup::ObjectInteractions;
			case EntityType::Crusher: return (CollisionGroup::Actor | CollisionGroup::Object | CollisionGroup::AllGeometry);
			//case EntityType::Light: return CollisionGroup::None;
			//case EntityType::Pivot: return CollisionGroup::All;
			//case EntityType::Bone: return CollisionGroup::None;
			case EntityType::Object: return CollisionGroup::ObjectInteractions;
			case EntityType::Player: return CollisionGroup::PlayerInteractions;
			//case EntityType::Camera: return CollisionGroup::ObjectInteractions;
			case EntityType::Enemy: return CollisionGroup::EnemyInteractions;
			case EntityType::FriendlyActor: return CollisionGroup::ObjectInteractions;
			case EntityType::Collectable: return CollisionGroup::CollectableInteractions;
			case EntityType::Particle: return CollisionGroup::AllGeometry;
			case EntityType::Projectile: return CollisionGroup::HitDetectionInteractions;
			case EntityType::EventTrigger: return CollisionGroup::Actor;

			case EntityType::WaterZone: return CollisionGroup::All;
			case EntityType::KillZone: return CollisionGroup::All;
			case EntityType::DamageZone: return CollisionGroup::All;

			case EntityType::Generator: return CollisionGroup::None;
		}

		return CollisionGroup::None;
	}

	CollisionConfig::CollisionConfig(EntityType type) :
		group(resolve_collision_group(type)),
		solid_mask(resolve_solid_mask(type)),
		interaction_mask(resolve_interaction_mask(type))
	{}

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

	std::unique_ptr<btCollisionObject> CollisionComponent::make_collision_object(const CollisionConfig& config)
	{
		auto obj = std::make_unique<btCollisionObject>();

		using native_flags = btCollisionObject::CollisionFlags;
		using enum native_flags;

		std::underlying_type_t<native_flags> flags = 0;

		if ((config.group & CollisionGroup::StaticGeometry))
		{
			// Bullet does not allow a collision object to be both static and dynamic.
			assert(!(config.group & CollisionGroup::DynamicGeometry));

			flags |= CF_STATIC_OBJECT;
		}

		// No other flags are used at the moment.

		obj->setCollisionFlags(static_cast<native_flags>(flags));

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