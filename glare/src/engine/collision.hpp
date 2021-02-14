#pragma once

#include "types.hpp"

//#include <climits>

class btCollisionShape;
class btCollisionObject;

namespace engine
{
	class World;
	class PhysicsSystem;

	enum class CollisionShape
	{
		Box = 1,
		Capsule = 2,
		//Sphere = 3,
	};

	enum class CollisionMask : std::uint32_t
	{
		None = 0,

		// Bit 1 is reserved.

		LevelGeometry = (1 << 1),
		Player = (1 << 2),
		Objects = (1 << 3),
		Collectable = (1 << 4),
		Water = (1 << 5),

		DamageZone = (1 << 6),
		KillZone = (1 << 7),
		
		EventTrigger = (1 << 8),

		StandardObject = (LevelGeometry|Player|Objects),

		All = (UINT32_MAX),
	};

	FLAG_ENUM(std::uint32_t, CollisionMask);

	struct CollisionComponent
	{
		public:
			friend class PhysicsSystem;

			using RawShape = btCollisionShape;
			using Shape = std::shared_ptr<RawShape>;

			CollisionComponent(const Shape& shape, float mass=0.0f, CollisionMask interaction_mask=CollisionMask::None, CollisionMask solid_mask=CollisionMask::None, bool auto_activate=true);
			~CollisionComponent();

			CollisionComponent(CollisionComponent&&) = default;
			CollisionComponent& operator=(CollisionComponent&&) = default;

			CollisionComponent(const CollisionComponent&) = delete;
			CollisionComponent& operator=(const CollisionComponent&) = delete;

			inline CollisionMask get_interactions() const { return interaction_mask; }
			inline CollisionMask get_solids() const { return solid_mask; }

			inline CollisionMask get_full_mask() const { return (get_interactions() | get_solids()); }

			float mass = 0.0f;
			std::unique_ptr<btCollisionObject> collision;
		protected:
			void activate(bool force = false);
			//void deactivate();

			Shape shape;

			CollisionMask interaction_mask = CollisionMask::None;
			CollisionMask solid_mask = CollisionMask::None;
	};

	Entity attach_collision(World& world, Entity entity, const CollisionComponent::Shape& collision_data, float mass = 0.0f, CollisionMask interaction_mask = CollisionMask::None, CollisionMask solid_mask = CollisionMask::None);
}