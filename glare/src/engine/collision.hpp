#pragma once

#include "types.hpp"

#include <utility>
#include <variant>
#include <type_traits>
//#include <optional>

//#include <tuple>

//#include <climits>

#include <bullet/btBulletCollisionCommon.h>
//#include <bullet/BulletCollision/CollisionDispatch/btCollisionObject.h>

class btCollisionShape;
class btConcaveShape;
class btConvexShape;
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

	/*
		Represents a 'collider' object attached to an entity.
		This can be thought of as an in-world instance of collision shape (see 'Shape' types below),
		tied directly to an Entity's world-space transform.

		***To attach `CollisionComponent` objects to an entity, please use the `attach_collision` command to
		ensure registration with the `World` object's physics-system occurs properly.***
	*/
	struct CollisionComponent : protected CollisionConfig
	{
		public:
			friend class PhysicsSystem;

			using RawShape = btCollisionShape;
			using ConcaveShapeRaw = btConcaveShape;
			using ConvexShapeRaw = btConvexShape;

			using Shape        = std::shared_ptr<RawShape>;
			using ConcaveShape = std::shared_ptr<ConcaveShapeRaw>;
			using ConvexShape  = std::shared_ptr<ConvexShapeRaw>;

			//CollisionComponent(const Shape& shape, const CollisionConfig& config, float mass = 0.0f, bool auto_activate=true);
		protected:
			inline CollisionComponent(const CollisionConfig& config, float mass) :
				CollisionConfig(config),
				mass(mass),
				collision(make_collision_object())
			{}
		public:
			
			/*
			* For some reason, this template wasn't working:
			template <typename ShapeType>
			inline CollisionComponent(const ShapeType& shape, const CollisionConfig& config, float mass, bool auto_activate) :
				CollisionComponent(config, mass)
			{
				if (shape)
				{
					set_shape(shape);

					if (auto_activate)
					{
						activate();
					}
				}
			}
			*/

			inline CollisionComponent(const Shape& shape, const CollisionConfig& config, float mass, bool auto_activate=true) :
				CollisionComponent(config, mass)
			{
				if (shape)
				{
					set_shape(shape);

					if (auto_activate)
					{
						activate();
					}
				}
			}

			inline CollisionComponent(const ConcaveShape& shape, const CollisionConfig& config, float mass, bool auto_activate=true) :
				CollisionComponent(config, mass)
			{
				if (shape)
				{
					set_shape(shape);

					if (auto_activate)
					{
						activate();
					}
				}
			}

			inline CollisionComponent(const ConvexShape& shape, const CollisionConfig& config, float mass, bool auto_activate=true) :
				CollisionComponent(config, mass)
			{
				if (shape)
				{
					set_shape(shape);

					if (auto_activate)
					{
						activate();
					}
				}
			}


			~CollisionComponent();

			CollisionComponent(CollisionComponent&&) = default;
			CollisionComponent& operator=(CollisionComponent&&) = default;

			CollisionComponent(const CollisionComponent&) = delete;
			CollisionComponent& operator=(const CollisionComponent&) = delete;

			inline CollisionGroup get_group() const { return group; }

			inline CollisionGroup get_interactions() const { return interaction_mask; }
			inline CollisionGroup get_solids() const { return solid_mask; }

			inline CollisionGroup get_full_mask() const { return (get_interactions() | get_solids()); }

			Shape get_shape() const;

			ConvexShape get_convex_shape() const;
			ConcaveShape get_concave_shape() const;

			void set_shape(const Shape& shape);
			void set_shape(const ConcaveShape& shape);
			void set_shape(const ConvexShape& shape);
		protected:
			static std::unique_ptr<btCollisionObject> make_collision_object();
			
			float mass = 0.0f;

			std::unique_ptr<btCollisionObject> collision;

			std::variant<Shape, ConcaveShape, ConvexShape> shape;

			void activate(bool force = false);
			//void deactivate();

	};

	Entity attach_collision_impl(World& world, Entity entity, CollisionComponent&& col);

	template <typename ShapeType>
	inline Entity attach_collision(World& world, Entity entity, const ShapeType& collision_data, const CollisionConfig& config, float mass=0.0f) // CollisionComponent::Shape
	{
		return attach_collision_impl(world, entity, CollisionComponent(collision_data, config, mass));
	}
}