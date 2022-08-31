#pragma once

#include "collision_group.hpp"
#include "collision_config.hpp"
#include "collision_motion_state.hpp"
#include "collision_body_type.hpp"

#include "types.hpp"

#include <util/optional.hpp>

#include <utility>
#include <variant>
#include <type_traits>
#include <optional>
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

	//class CollisionMotionState;

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

			using RawShape        = btCollisionShape;
			using ConcaveShapeRaw = btConcaveShape;
			using ConvexShapeRaw  = btConvexShape;

			//using NativeCollisionObject = btCollisionObject;

			using Shape        = std::shared_ptr<RawShape>;
			using ConcaveShape = std::shared_ptr<ConcaveShapeRaw>;
			using ConvexShape  = std::shared_ptr<ConvexShapeRaw>;

			//CollisionComponent(const Shape& shape, const CollisionConfig& config, float mass = 0.0f, bool auto_activate=true);
		protected:
			// The return-value of this function states whether the object in question has been flagged as kinematic.
			// TODO: Look into making the return-value of this function a struct outlining several properties of the collision-object.
			static bool apply_collision_flags(btCollisionObject& c_obj, const CollisionConfig& config, bool keep_existing_flags=true, bool allow_kinematic=true);

			// This constructor overload does not generate an internal collision object. (To do so requires a collision-shape; see other overloads)
			// This exists purely to simplify constructor delegation.
			CollisionComponent(const CollisionConfig& config, float mass, std::unique_ptr<CollisionMotionState>&& motion_state={});

			// Acceptable inputs for `set_shape`/`set_shape_impl`:

			// Sets the internal collision-shape instance to a non-specific `Shape` type.
			void set_shape(const Shape& shape);

			// Sets the internal collision-shape instance to a concave shape.
			void set_shape(const ConcaveShape& shape);

			// Sets the internal collision-shape instance to a convex shape.
			void set_shape(const ConvexShape& shape);
		private:
			// Implementation for `set_shape`.
			template <typename ShapeRefType>
			void set_shape_impl(ShapeRefType&& shape);

			void set_collision_object(std::unique_ptr<btCollisionObject>&& collision);

			void build_basic_collision_object(const CollisionConfig& config); // , float mass

			// The `collision_shape` object specified must be a raw pointer to the `shape` object's underlying value. (i.e. from `shape.get()`)
			// This function returns `true` if the internally generated rigid-body is kinematic.
			// TODO: Look into making the return-value of this function a struct outlining several properties of the collision-object. (see `apply_collision_flags`)
			bool build_rigid_body(const CollisionConfig& config, btCollisionShape* collision_shape, float mass, bool allow_kinematics=true);
		public:
			template <typename ShapeRefType> // Shape, ConvexShape, ConcaveShape
			inline CollisionComponent(const ShapeRefType& shape, const CollisionConfig& config, float mass, std::optional<CollisionBodyType> body_type_in=std::nullopt, std::unique_ptr<CollisionMotionState>&& motion_state_in={}, bool auto_activate=true) :
				CollisionComponent(config, mass, std::move(motion_state_in))
			{
				auto body_type = body_type_in.value_or(CollisionBodyType::Basic);

				// Check for the basic collision-object case:
				if (body_type == CollisionBodyType::Basic)
				{
					build_basic_collision_object(config);

					// `shape` needs to be specified after construction for basic collision-objects.
					set_shape(shape);
				}
				else
				{
					// Check for all other body types:
					switch (body_type)
					{
						case CollisionBodyType::Static:
						case CollisionBodyType::Kinematic:
						{
							// TODO: Determine if `shape` setup code is shared between cases:
							assert(shape);

							// `shape` must be specified during/before construction for rigid bodies.
							set_shape(shape);

							// Note: `this->shape` needs to be defined before this is called.
							bool is_kinematic = build_rigid_body(config, shape.get(), mass, !(body_type & CollisionBodyType::Static));

							// Assert that we're either not kinematic, or if we are, that `motion_state` exists.
							assert((!(is_kinematic) || (motion_state != nullptr)));

							break;
						}

						case CollisionBodyType::Ghost:
							// TODO: Implement ghost-objects.
							assert(false);

							break;
					}
				}

				if (auto_activate)
				{
					activate();
				}
			}

			CollisionComponent(CollisionComponent&&) = default;
			CollisionComponent& operator=(CollisionComponent&&) = default;

			CollisionComponent(const CollisionComponent&) = delete;
			CollisionComponent& operator=(const CollisionComponent&) = delete;

			inline CollisionGroup get_group() const { return group; }

			inline CollisionGroup get_interactions() const { return interaction_mask; }
			inline CollisionGroup get_solids() const { return solid_mask; }

			// Returns the internally bound collision-shape object, regardless of concave/convex qualification.
			Shape get_shape() const;

			// Retrieves the internally bound convex collision-shape object (if available).
			ConvexShape get_convex_shape() const;

			// Retrieves the internally bound concave collision-shape object (if available).
			ConcaveShape get_concave_shape() const;

			btCollisionObject* get_collision_object();
			const btCollisionObject* get_collision_object() const;
			
			// TODO: May implement this later. (would require `std::variant` usage like `shape`)
			//btRigidBody* get_rigid_body();
			//const btRigidBody* get_rigid_body() const;

			CollisionMotionState* get_motion_state();
			const CollisionMotionState* get_motion_state() const;

			inline bool has_motion_state() const { return get_motion_state(); }

			// Returns `true` if activation was successful.
			bool activate(bool force=false);

			// Returns `true` if deactivation was successful.
			bool deactivate(bool force=false);

			bool is_active() const;
			bool is_kinematic() const;
		protected:
			float mass = 0.0f;

			std::unique_ptr<CollisionMotionState> motion_state;
			std::unique_ptr<btCollisionObject> collision;

			std::variant<Shape, ConcaveShape, ConvexShape> shape;
	};

	Entity attach_collision_impl(World& world, Entity entity, CollisionComponent&& col);

	template <typename ShapeType>
	inline Entity attach_collision(World& world, Entity entity, const ShapeType& collision_shape_data, const CollisionConfig& config, float mass=0.0f) // CollisionComponent::Shape
	{
		// TODO: Refactor into something more explicit from the user. (or something better for automatically determining body type)
		// We currently assume kinematic rigid bodies if a motion-state is generated:
		auto motion_state = make_collision_motion_state(world, entity, config);
		auto body_type = util::optional_if(static_cast<bool>(motion_state), CollisionBodyType::Kinematic);

		return attach_collision_impl(world, entity, CollisionComponent(collision_shape_data, config, mass, body_type, std::move(motion_state)));
	}
}