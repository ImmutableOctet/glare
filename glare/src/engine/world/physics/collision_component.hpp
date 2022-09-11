#pragma once

#include "collision_group.hpp"
#include "collision_config.hpp"
#include "collision_motion_state.hpp"
#include "collision_body_type.hpp"
#include "collision_cast_method.hpp"

#include "types.hpp"

#include <util/optional.hpp>

#include <utility>
#include <variant>
#include <type_traits>
#include <optional>
#include <memory>
//#include <tuple>
//#include <climits>

// Bullet's included in the header for now. This is due to some issues
// with missing destructors for the `unique_ptr` instances.
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

namespace engine
{
	class World;
	class PhysicsSystem;

	//class CollisionMotionState;

	// `set_entity_for_collision_object` is considered an internal implementation detail and is therefore inaccessible.
	//void set_entity_for_collision_object(btCollisionObject& c_obj, Entity entity);

	// Returns the `Entity` instance associated with the specified Bullet collision-object.
	// It is only legal to call this function on collision-objects managed by a `CollisionComponent`.
	Entity get_entity_from_collision_object(const btCollisionObject& c_obj);

	void update_collision_object_transform(btCollisionObject& obj, const math::Matrix& m);

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

			// Shorthand type aliases:
			using RawShape        = CollisionRaw;             // btCollisionShape;
			using ConvexShapeRaw  = ConvexCollisionShapeRaw;  // btConvexShape;
			using ConcaveShapeRaw = ConcaveCollisionShapeRaw; // btConcaveShape;

			using Shape           = CollisionShape;        // std::shared_ptr<RawShape>;
			using ConvexShape     = ConvexCollisionShape;  // std::shared_ptr<ConvexShapeRaw>;
			using ConcaveShape    = ConcaveCollisionShape; // std::shared_ptr<ConcaveShapeRaw>;

			// Specific to this component type:
			using CollisionObject = std::unique_ptr<btCollisionObject>;
			using RigidBody       = std::unique_ptr<btRigidBody>;

			using collision_object_variant_t = std::variant<std::monostate, CollisionObject, RigidBody>;
			using shape_variant_t = std::variant<Shape, ConcaveShape, ConvexShape>;
		protected:
			static constexpr std::size_t null_collision_object_index = 0; // AKA, what index has `std::monostate`.

			// The return-value of this function states whether the object in question has been flagged as kinematic.
			// TODO: Look into making the return-value of this function a struct outlining several properties of the collision-object.
			static bool apply_collision_flags(btCollisionObject& c_obj, const CollisionConfig& config, bool keep_existing_flags=true, bool allow_kinematic=true);

			// This constructor overload does not generate an internal collision object. (To do so requires a collision-shape; see other overloads)
			// This exists purely to simplify constructor delegation.
			CollisionComponent(const CollisionConfig& config, std::unique_ptr<CollisionMotionState>&& motion_state={});

			bool collision_object_in_monostate() const;
			const collision_object_variant_t& get_collision_object_variant() const;
		private:
			// Declared for internal use only. (Forwards to static free-function implementation)
			static void set_entity_for_collision_object(btCollisionObject& c_obj, Entity entity);

			template <typename RawShapeType>
			inline void set_shape(const std::shared_ptr<RawShapeType>& shape)
			{
				if (!shape)
				{
					return;
				}

				auto* collision = get_collision_object();

				if (collision)
				{
					collision->setCollisionShape(shape.get());
				}

				// Enforce correct template usage and variant assignment:

				// Static scenarios (Convex and concave):
				if constexpr (std::is_base_of_v<ConcaveShapeRaw, RawShapeType>)
				{
					this->shape = std::static_pointer_cast<ConcaveShape>(shape);
				}
				else if constexpr (std::is_base_of_v<ConvexShapeRaw, RawShapeType>)
				{
					this->shape = std::static_pointer_cast<ConvexShape>(shape);
				}
				// Dynamic scenario (any input shape):
				else
				{
					if (auto convex_ptr = std::dynamic_pointer_cast<ConvexShapeRaw>(shape); convex_ptr)
					{
						this->shape = std::move(convex_ptr);
					}
					else if (auto concave_ptr = std::dynamic_pointer_cast<ConcaveShapeRaw>(shape); concave_ptr)
					{
						this->shape = std::move(concave_ptr);
					}
					else
					{
						this->shape = std::static_pointer_cast<RawShape>(shape);
					}
				}
			}

			template <typename CollisionObjectType>
			inline void set_collision_object(CollisionObjectType&& collision)
			{
				if (collision)
				{
					// For safety, we always set the associated `Entity` to `null` by default.
					set_entity_for_collision_object(*collision, null);

					this->collision_object = std::move(collision);
				}
			}

			void build_basic_collision_object(const CollisionConfig& config); // , float mass

			// The `collision_shape` object specified must be a raw pointer to the `shape` object's underlying value. (i.e. from `shape.get()`)
			// This function returns `true` if the internally generated rigid-body is kinematic.
			// TODO: Look into making the return-value of this function a struct outlining several properties of the collision-object. (see `apply_collision_flags`)
			bool build_rigid_body(const CollisionConfig& config, btCollisionShape* collision_shape, float mass, bool allow_kinematics=true);

			// Generates a default collision-cast method, given the configuration of the component.
			CollisionCastMethod default_kinematic_cast_method() const;
		public:
			// This calls the free-function of the same name.
			static Entity get_entity_from_collision_object(const btCollisionObject& c_obj);

			template <typename ShapeRefType> // Shape, ConvexShape, ConcaveShape
			inline CollisionComponent
			(
				const ShapeRefType& shape,
				const CollisionConfig& config,
				float mass,
				std::optional<CollisionBodyType> body_type_in=std::nullopt,
				std::unique_ptr<CollisionMotionState>&& motion_state_in={},
				std::optional<CollisionCastMethod> kinematic_cast_method_in=std::nullopt,
				bool activate_immediately=true
			):
				CollisionComponent(config, std::move(motion_state_in))
			{
				auto body_type = body_type_in.value_or(CollisionBodyType::Basic);

				// Check for all other body types:
				switch (body_type)
				{
					case CollisionBodyType::Basic:
						build_basic_collision_object(config);

						// `shape` needs to be specified after construction for basic collision-objects.
						set_shape(shape);

						break;
					case CollisionBodyType::Ghost:
						// TODO: Implement ghost-objects.
						assert(false);

						break;
					default:
						// TODO: Determine if `shape` setup code is shared between cases:
						assert(shape);

						auto* shape_ptr = shape.get();

						// `shape` must be specified during/before construction for rigid bodies.
						set_shape(shape);

						// Note: `this->shape` needs to be defined before this is called.
						build_rigid_body(config, shape_ptr, mass, (body_type != CollisionBodyType::Static));

						break;
				}

				// If-statement used in place of `value_or` due to possible overhead
				// incurred from calling `default_kinematic_cast_method`.
				if (kinematic_cast_method_in.has_value())
				{
					kinematic_cast_method = *kinematic_cast_method_in;
				}
				else
				{
					kinematic_cast_method = default_kinematic_cast_method();
				}

				if (activate_immediately)
				{
					activate();
				}
				else
				{
					// Bullet has a built-in auto-activate setting,
					// so we ensure deactivation here.
					deactivate();
				}
			}

			CollisionComponent(CollisionComponent&&) = default;
			CollisionComponent& operator=(CollisionComponent&&) = default;

			CollisionComponent(const CollisionComponent&) = delete;
			CollisionComponent& operator=(const CollisionComponent&) = delete;

			// Manually updates the transformation matrix of the internal held collision-object.
			// This does not directly affect the Entity's `TransformComponent`.
			void update_transform(const math::Matrix& tform);

			inline CollisionGroup get_group() const        { return group; }
			inline CollisionGroup get_interactions() const { return interaction_mask; }
			inline CollisionGroup get_solids() const       { return solid_mask; }

			// Returns the internally bound collision-shape object, regardless of concave/convex qualification.
			Shape get_shape() const;
			RawShape* peek_shape() const;

			// Retrieves the internally bound convex collision-shape object (if available).
			ConvexShape get_convex_shape() const;
			ConvexShapeRaw* peek_convex_shape() const;

			// Retrieves the internally bound concave collision-shape object (if available).
			ConcaveShape get_concave_shape() const;
			ConcaveShapeRaw* peek_concave_shape() const;

			btCollisionObject* get_collision_object();
			const btCollisionObject* get_collision_object() const;
			
			btRigidBody* get_rigid_body();
			const btRigidBody* get_rigid_body() const;

			CollisionMotionState* get_motion_state();
			const CollisionMotionState* get_motion_state() const;

			template <typename callback_fn>
			inline bool on_collision_object(callback_fn&& callback) const
			{
				bool executed = false;

				std::visit
				(
					[&callback, &executed](const auto& obj)
					{
						using obj_t = std::decay_t<decltype(obj)>;

						if constexpr (!std::is_same_v<obj_t, std::monostate>)
						{
							if (static_cast<bool>(obj))
							{
								callback(std::forward<decltype(obj)>(obj)); // obj_t

								executed = true;
							}
						}
					},

					this->collision_object
				);

				return executed;
			}

			// Returns `true` if activation was successful.
			bool activate(bool force=false);

			// Returns `true` if deactivation was successful.
			bool deactivate(bool force=false);

			bool is_active() const;
			bool is_kinematic() const;
			bool is_dynamic() const;
			bool is_static() const;

			bool has_collision_object() const;
			bool has_motion_state() const;

			float get_mass() const;
			void set_mass(float mass);

			inline CollisionCastMethod get_kinematic_cast_method() const { return kinematic_cast_method; }
		protected:
			CollisionCastMethod kinematic_cast_method = CollisionCastMethod::None;

			std::unique_ptr<CollisionMotionState> motion_state;
			collision_object_variant_t collision_object;
			shape_variant_t shape;
	};

	Entity attach_collision_impl(World& world, Entity entity, CollisionComponent&& col);

	template <typename ShapeType>
	inline Entity attach_collision
	(
		World& world, Entity entity,
		const ShapeType& collision_shape_data,
		const CollisionConfig& config,
		float mass=0.0f,
		std::optional<CollisionCastMethod> cast_method=std::nullopt
	) // CollisionComponent::Shape
	{
		// TODO: Refactor into something more explicit from the user. (or something better for automatically determining body type)
		// We currently assume kinematic rigid bodies if a motion-state is generated:
		std::unique_ptr<CollisionMotionState> motion_state;

		CollisionBodyType body_type = get_collision_body_type(config.group);

		switch (body_type)
		{
			//case CollisionBodyType::Kinematic:
			case CollisionBodyType::Dynamic:
				motion_state = make_collision_motion_state(world, entity, config);

				break;
		}

		return attach_collision_impl(world, entity, CollisionComponent(collision_shape_data, config, mass, body_type, std::move(motion_state), cast_method));
	}
}