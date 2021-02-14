#include "collision.hpp"
#include "world/world.hpp"
#include "types.hpp"

#include <math/bullet.hpp>

//#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/BulletCollision/btBulletCollisionCommon.h>

#include <utility>

namespace engine
{
	CollisionComponent::CollisionComponent(const CollisionComponent::Shape& shape, float mass, CollisionMask interaction_mask, CollisionMask solid_mask, bool auto_activate) :
		mass(mass),

		interaction_mask(interaction_mask),
		solid_mask(solid_mask),

		shape(shape),
		collision(nullptr)
	{
		if (shape != nullptr)
		{
			collision = std::make_unique<btCollisionObject>();
			collision->setCollisionShape(shape.get());

			if (auto_activate)
			{
				activate();
			}
		}
	}

	CollisionComponent::~CollisionComponent() {}

	void CollisionComponent::activate(bool force)
	{
		collision->activate(force);
	}
	
	Entity attach_collision(World& world, Entity entity, const CollisionComponent::Shape& collision_data, float mass, CollisionMask interaction_mask, CollisionMask solid_mask)
	{
		auto& registry = world.get_registry();

		auto& component = registry.emplace<CollisionComponent>(entity, collision_data, mass, interaction_mask, solid_mask);

		world.on_new_collider({ entity });

		return entity;
	}
}