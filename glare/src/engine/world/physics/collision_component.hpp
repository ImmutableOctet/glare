#pragma once

#include "collision_group.hpp"
#include "collision_config.hpp"

#include "types.hpp"

#include <utility>
#include <variant>
#include <type_traits>
//#include <optional>
//#include <tuple>
//#include <climits>

// Bullet's included in the header for now. This is due to some issues
// with missing destructors for the `unique_ptr` instances.
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

	// Returns the `Entity` instance associated with the specified Bullet collision-object.
	// It is only legal to call this function on collision-objects managed by a `CollisionComponent`.
	Entity get_entity_from_collision_object(const btCollisionObject& c_obj);

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

			//using NativeCollisionObject = btCollisionObject;

			using Shape        = std::shared_ptr<RawShape>;
			using ConcaveShape = std::shared_ptr<ConcaveShapeRaw>;
			using ConvexShape  = std::shared_ptr<ConvexShapeRaw>;

			//CollisionComponent(const Shape& shape, const CollisionConfig& config, float mass = 0.0f, bool auto_activate=true);
		protected:
			inline CollisionComponent(const CollisionConfig& config, float mass) :
				CollisionConfig(config),
				mass(mass),
				collision(make_collision_object(config, mass))
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

			inline btCollisionObject* get_collision_object();
			inline const btCollisionObject* get_collision_object() const;
		protected:
			static std::unique_ptr<btCollisionObject> make_collision_object(const CollisionConfig& config, float mass);
			
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