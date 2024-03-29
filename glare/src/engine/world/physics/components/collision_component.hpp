#pragma once

#include <engine/world/physics/collision_group.hpp>
#include <engine/world/physics/collision_config.hpp>
#include <engine/world/physics/collision_motion_state.hpp>
#include <engine/world/physics/collision_body_type.hpp>
#include <engine/world/physics/collision_cast_method.hpp>
#include <engine/world/physics/kinematic_resolution_config.hpp>

#include "types.hpp"

#include <util/optional.hpp>

#include <utility>
#include <variant>
#include <type_traits>
#include <optional>
#include <memory>
#include <tuple>
//#include <climits>

// Bullet's included in the header for now. This is due to some issues
// with missing destructors for the `unique_ptr` instances.
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

namespace engine
{
	class World;
	class PhysicsSystem;

	struct CollisionShapeDescription;

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

			// Declared for internal use only. (Forwards to static free-function implementation)
			static void set_entity_for_collision_object(btCollisionObject& c_obj, Entity entity);

			// This constructor overload does not generate an internal collision object. (To do so requires a collision-shape; see other overloads)
			// This exists purely to simplify constructor delegation.
			CollisionComponent(const CollisionConfig& config, std::unique_ptr<CollisionMotionState>&& motion_state={});

			bool collision_object_in_monostate() const;
			const collision_object_variant_t& get_collision_object_variant() const;

		private:
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
					// NOTE: Dynamic casts disabled due to RTTI-related issue with Bullet package.
					
					//if (auto convex_ptr = std::dynamic_pointer_cast<ConvexShapeRaw>(shape); convex_ptr)
					if (shape->isConvex())
					{
						auto convex_ptr = std::static_pointer_cast<ConvexShapeRaw>(shape);
						this->shape = std::move(convex_ptr);
					}
					//else if (auto concave_ptr = std::dynamic_pointer_cast<ConcaveShapeRaw>(shape); concave_ptr)
					else if (shape->isConcave())
					{
						auto concave_ptr = std::static_pointer_cast<ConcaveShapeRaw>(shape);
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
			KinematicResolutionConfig default_kinematic_resolution_method() const;
		public:
			// This calls the free-function of the same name.
			static Entity get_entity_from_collision_object(const btCollisionObject& c_obj);

			CollisionComponent
			(
				const CollisionShapeDescription& shape_details,
				const CollisionConfig& config,
				std::optional<KinematicResolutionConfig> kinematic_resolution=std::nullopt,
				std::optional<CollisionBodyType> body_type=std::nullopt,
				float mass = 0.0f,
				std::unique_ptr<CollisionMotionState>&& motion_state_in={},
				bool activate_immediately=true
			);

			template <typename ShapeRefType> // Shape, ConvexShape, ConcaveShape
			CollisionComponent
			(
				const ShapeRefType& shape,
				const CollisionConfig& config,
				std::optional<KinematicResolutionConfig> kinematic_resolution=std::nullopt,
				std::optional<CollisionBodyType> body_type=std::nullopt,
				float mass=0.0f,
				std::unique_ptr<CollisionMotionState>&& motion_state_in={},
				bool activate_immediately=true
			):
				CollisionComponent(config, std::move(motion_state_in))
			{
				if (!body_type)
				{
					body_type = get_collision_body_type(config.group);
				}

				// Check for all other body types:
				switch (*body_type)
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
				// incurred from calling `default_kinematic_resolution_method`.
				if (kinematic_resolution.has_value())
				{
					this->kinematic_resolution = kinematic_resolution;
				}
				else
				{
					this->kinematic_resolution = default_kinematic_resolution_method();
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

			CollisionComponent(CollisionComponent&&) noexcept = default;
			CollisionComponent& operator=(CollisionComponent&&) noexcept = default;

			CollisionComponent(const CollisionComponent&) = delete;
			CollisionComponent& operator=(const CollisionComponent&) = delete;

			// Manually updates the transformation matrix of the internal held collision-object.
			// This does not directly affect the Entity's `TransformComponent`.
			void update_transform(const math::Matrix& tform);

			math::Matrix get_collision_object_transform() const;
			math::Matrix3x3 get_collision_object_orientation() const;

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
			bool is_convex() const;

			bool has_collision_object() const;
			bool has_motion_state() const;

			float get_mass() const;
			void set_mass(float mass);

			bool has_kinematic_cast_method() const;

			// Returns the 'min' and 'max' points for the axis-aligned bounding-box
			// (AABB) of the internal collision-object in world-space.
			std::tuple<math::Vector, math::Vector> get_world_aabb() const;

			// Returns the center-point of the internal collision-object in world-space
			// as well as a radius representing the object's spherical bounds.
			std::tuple<math::Vector, float> get_world_bounding_sphere() const;

			// Retrieves lengths for each of the AABB axes.
			// (max_aabb - min_aabb)
			math::Vector get_aabb_lengths() const;

			// Retrieves the size of the internal collision-object as an AABB.
			// NOTE: The value returned is subject to padding from Bullet.
			float get_aabb_size() const;

			// Retrieves the bounding-sphere radius of the internal collision-object.
			// NOTE: The value returned is subject to padding from Bullet.
			float get_bounding_radius() const;

			/*
				Retrieves the average length of all three AABB axes, divided by two.
				(((length_x+length_y+length_z)/2)/3), where lengths are: (max_aabb-min_aabb)
				
				This effectively returns the largest spherical radius that could exist within the AABB,
				as opposed to `get_bounding_radius`, which returns the smallest radius that could encapsulate the AABB.
				
				NOTE: The value returned is subject to padding from Bullet.

				See also: `get_aabb_lengths`
			*/
			float get_inner_radius() const;

			// See notes for `get_inner_radius`. (2x)
			float get_inner_diameter() const;

			CollisionCastMethod get_kinematic_cast_method() const;

			void set_kinematic_resolution(const KinematicResolutionConfig& resolution);
			const std::optional<KinematicResolutionConfig>& get_kinematic_resolution() const;
			std::optional<KinematicResolutionConfig>& get_kinematic_resolution();

		protected:
			std::optional<KinematicResolutionConfig> kinematic_resolution;

			std::unique_ptr<CollisionMotionState> motion_state;
			collision_object_variant_t collision_object;
			shape_variant_t shape;
	};
}