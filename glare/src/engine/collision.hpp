#pragma once

#include "types.hpp"

//#include <tuple>

//#include <climits>

class btCollisionShape;
class btCollisionObject;

namespace engine
{
	class World;
	class PhysicsSystem;

	/*
	enum class CollisionShape
	{
		Box = 1,
		Capsule = 2,
		//Sphere = 3,
	};
	*/

	enum class CollisionGroup : std::uint32_t
	{
		All = (UINT32_MAX),

		None = 0,

		// Bit 1 is reserved.

		StaticGeometry   = (1 << 1),
		DynamicGeometry  = (1 << 2),
		Actor            = (1 << 3),
		Object           = (1 << 4),
		Bone             = (1 << 5),
		Zone             = (1 << 6),
		Particle         = (1 << 7),
		Projectile       = (1 << 8),

		Meta = (Zone), // | Bone
		AllGeometry = (StaticGeometry | DynamicGeometry),
		
		GeometrySolids = (All & ~(Meta)),
		ObjectSolids = (AllGeometry | Actor | Object),
		BoneSolids = ObjectSolids,
		ActorSolids = ObjectSolids,
		ProjectileSolids = (ObjectSolids | Projectile | Bone),


		PlayerInteractions      = All, // (All & ~(StaticGeometry)),

		ObjectInteractions      = (Actor | Object | Zone | AllGeometry),
		CollectableInteractions = (Actor | Zone),
		EnemyInteractions       = ObjectInteractions, // (Actor | Object | Zone),

		// Used for hitscan bullets, projectiles, etc.
		HitDetectionInteractions = (ObjectSolids | Zone | Bone),
	};

	FLAG_ENUM(std::uint32_t, CollisionGroup);

	//using CollisionMask = CollisionGroup;

	struct CollisionConfig
	{
		CollisionGroup group = CollisionGroup::None;
		CollisionGroup solid_mask = CollisionGroup::None;
		CollisionGroup interaction_mask = CollisionGroup::None;

		static CollisionGroup resolve_collision_group(EntityType type);
		static CollisionGroup resolve_solid_mask(EntityType type);
		static CollisionGroup resolve_interaction_mask(EntityType type);

		//using CollisionLookupResult = std::tuple<CollisionGroup, CollisionGroup, CollisionGroup>;

		CollisionConfig() = default;
		CollisionConfig(const CollisionConfig&) = default;
		CollisionConfig(CollisionConfig&&) = default;

		CollisionConfig& operator=(CollisionConfig&&) = default;

		CollisionConfig(EntityType type);
	};

	struct CollisionComponent : protected CollisionConfig
	{
		public:
			friend class PhysicsSystem;

			using RawShape = btCollisionShape;
			using Shape = std::shared_ptr<RawShape>;

			CollisionComponent(const Shape& shape, const CollisionConfig& config, float mass = 0.0f, bool auto_activate=true);
			~CollisionComponent();

			CollisionComponent(CollisionComponent&&) = default;
			CollisionComponent& operator=(CollisionComponent&&) = default;

			CollisionComponent(const CollisionComponent&) = delete;
			CollisionComponent& operator=(const CollisionComponent&) = delete;

			inline CollisionGroup get_group() const { return group; }

			inline CollisionGroup get_interactions() const { return interaction_mask; }
			inline CollisionGroup get_solids() const { return solid_mask; }

			inline CollisionGroup get_full_mask() const { return (get_interactions() | get_solids()); }

			void set_shape(const Shape& shape);

		protected:
			float mass = 0.0f;

			std::unique_ptr<btCollisionObject> collision;
			Shape shape;

			void activate(bool force = false);
			//void deactivate();

	};

	Entity attach_collision(World& world, Entity entity, const CollisionComponent::Shape& collision_data, const CollisionConfig& config, float mass=0.0f);
}