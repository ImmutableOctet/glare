#pragma once

#include "types.hpp"

//#include <climits>

class btCollisionShape;
class btCollisionObject;

namespace engine
{
	class World;

	enum class CollisionShape
	{
		Box = 1,
		Capsule = 2,
		//Sphere = 3,
	};

	enum class CollisionMask : std::uint32_t
	{
		None = 0,

		LevelGeometry = (1 << 0),
		Player = (1 << 1),
		Objects = (1 << 2),
		Collectable = (1 << 3),
		Water = (1 << 4),

		DamageZone = (1 << 5),
		KillZone = (1 << 6),
		
		EventTrigger = (1 << 7),

		StandardObject = (LevelGeometry|Player|Objects),

		All = (UINT32_MAX),
	};

	FLAG_ENUM(std::uint32_t, CollisionMask);

	struct CollisionComponent
	{
		public:
			CollisionComponent(const std::shared_ptr<btCollisionShape>& shape, float mass=0.0f, CollisionMask interaction_mask=CollisionMask::None, CollisionMask solid_mask=CollisionMask::None, bool auto_activate=true);
			~CollisionComponent();

			CollisionComponent(CollisionComponent&&) = default;
			CollisionComponent& operator=(CollisionComponent&&) = default;

			CollisionComponent(const CollisionComponent&) = delete;
			CollisionComponent& operator=(const CollisionComponent&) = delete;

			float mass = 0.0f;
			std::unique_ptr<btCollisionObject> collision;

		protected:
			void activate(bool force = false);
			//void deactivate();

			std::shared_ptr<btCollisionShape> shape;

			CollisionMask interaction_mask = CollisionMask::None;
			CollisionMask solid_mask = CollisionMask::None;
	};

	Entity attach_collision(World& world, Entity entity, const std::shared_ptr<btCollisionShape>& collision_data, float mass = 0.0f, CollisionMask interaction_mask = CollisionMask::None, CollisionMask solid_mask = CollisionMask::None);
}