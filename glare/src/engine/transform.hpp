#pragma once

#include <optional>
#include <utility>

#include "types.hpp"
#include "relationship.hpp"

#include <math/math.hpp>

namespace engine
{
	// Forward declarations:
	class World;
	struct Transform;

	enum _TransformComponent_Dirty : std::uint8_t
	{
		None = 0,

		M                    = 1, // Model
		W                    = 2, // World
		IW                   = 4, // Inverse World
		
		/*
			Indicates that a change has taken place;
			used for collision, dynamic cubemap matrices (shadows, reflections), etc.
			
			Order of operations is essentially a mark-and-sweep approach:
			Modify Transform (flag triggered; mark) -> Detect flags -> Trigger events -> Disable flags (sweep) -> Repeat
		*/
		EventFlag = 8,

		All = (M | W | IW),
	};

	FLAG_ENUM(std::uint8_t, _TransformComponent_Dirty);

	// Data-only component; logic is stored in 'Transformation', as several operations require additional registry access.
	struct TransformComponent
	{
		public:
			using Dirty = _TransformComponent_Dirty;
			using Flag  = Dirty;
		protected:
			friend Transform;
			friend World;

			// Local transform:

			// Local position/translation.
			math::Vector translation = {};

			// Local scale/magnitude.
			math::Vector scale = { 1.0f, 1.0f, 1.0f }; // _s

			// Local orientation/basis.
			math::RotationMatrix basis = math::identity<math::RotationMatrix>(); // ('Basis') // _r

			// Model matrix.
			math::Matrix _m = math::affine_mat4(1.0f); // Model

			math::Matrix _w = math::identity<math::Matrix>(); // World
			math::Matrix _iw = glm::inverse(_w); // Inverse world

			Dirty _dirty = Dirty::All; // Dirty::None;

		public:
			bool invalid(Dirty flag) const;

			void invalidate(Dirty flag);
			Dirty validate(Dirty flag);

			// Helper function for event-flags, etc.
			template <typename Event>
			inline bool on_flag(Flag flag, Event&& event_fn)
			{
				if (invalid(flag))
				{
					validate(flag);

					event_fn();

					return true;
				}

				return false;
			}
	};

	struct TransformViewData
	{
		TransformViewData(Registry& registry, Entity entity);
		TransformViewData(Registry& registry, Entity entity, const Relationship& relationship);
		TransformViewData(Registry& registry, Entity entity, const Relationship& relationship, TransformComponent& transform);

		static std::optional<TransformViewData> get_parent_data(const TransformViewData& data);

		Registry& registry;

		// Entity:

		// 'relationship' being a local copy has side-effects, including both larger object size and some very rare edge-cases.
		Relationship relationship; // const Relationship&
		TransformComponent& transform;

		Entity entity;
	};

	struct Transform : public TransformViewData
	{
		public:
			static std::optional<Transform> get_transform_safe(Registry& registry, Entity entity);

			static math::RotationMatrix orientation(const math::Vector& origin, const math::Vector& target, const math::Vector& up={0.0f, 1.0f, 0.0f});
			static math::Quaternion quat_orientation(const math::Vector& origin, const math::Vector& target, const math::Vector& up={0.0f, 1.0f, 0.0f});
		protected:
			using Dirty = _TransformComponent_Dirty;

			// By storing a cached result of the 'relationship' and 'transform' components at construction of the child,
			// we are able to save time in the general case, where entity trees are <= 2 entities deep.
			// Additionally, this field assists in optimizing multiple requests for parent data, without incurring dynamic allocation.
			// Thanks to 'std::optional', this is also type and memory-safe within the scope of this type's utility.
			std::optional<TransformViewData> parent_data = std::nullopt;

			// Resolves 'parent_data' into a proper 'Transform' object, if applicable.
			std::optional<Transform> get_parent() const;

			Transform& invalidate();
			Transform& invalidate_world();

			// Updates the local 'model' matrix.
			math::Matrix& update_local_matrix(const math::Vector& translation, const math::Vector& scale, const math::RotationMatrix& basis);
			math::Matrix& update_local_matrix(bool force=false);

			math::Matrix& update_matrix(bool force=false);
			math::Matrix& update_inverse_matrix(bool force=false);

			bool invalid(Dirty flag) const;
			Dirty validate(Dirty flag);

			Transform& recalculate(bool force);

			Transform& apply_basis(const math::RotationMatrix& basis, bool local=false); // math::RotationMatrix&

			Transform(TransformViewData data);
		public:
			using Flag = Dirty;

			Transform(Registry& registry, Entity entity);
			Transform(Registry& registry, Entity entity, const Relationship& relationship);
			Transform(Registry& registry, Entity entity, const Relationship& relationship, TransformComponent& transform);

			~Transform();

			// Alias to helper function in 'TransformComponent'.
			/*
			template <typename Event>
			inline bool on_flag(Flag flag, Event&& event_fn)
			{
				return transform.on_flag(flag, std::forward<Event>(fn));
			}
			*/

			// TODO: Rename these routines to be more generic.
			// (i.e. collision is not the only use-case of event flags)
			bool collision_invalid() const;
			Transform& validate_collision();

			Transform& validate_collision_shallow();

			// Global transformations:
			math::Matrix get_matrix(bool force_refresh=false);
			math::Matrix get_inverse_matrix(bool force_refresh = false);
			math::Matrix get_camera_matrix();

			math::Vector get_position();
			math::Vector get_scale();
			math::RotationMatrix get_basis();

			// Retrieves the current rotation in the form of angles. (Pitch, Yaw, Roll)
			math::Vector get_rotation();

			// Retrieves the current local rotation in the form of angles. (Pitch, Yaw, Roll)
			math::Vector get_local_rotation();

			// Retrieves a normalized direction vector based on the transform's basis.
			math::Vector get_direction_vector(const math::Vector forward={0.0f, 0.0f, -1.0f});

			math::TransformVectors get_vectors(); // const

			inline float rx() { return get_rotation().x; }
			inline float ry() { return get_rotation().y; }
			inline float rz() { return get_rotation().z; }

			Transform& set_position(const math::Vector& position);
			Transform& set_scale(const math::Vector& scale);
			Transform& set_scale(float scale);

			Transform& set_basis(const math::RotationMatrix& basis);
			Transform& set_basis_q(const math::Quaternion& basis);

			// Euler angles. (Pitch, Yaw, Roll)
			Transform& set_rotation(const math::Vector& rv);
			Transform& set_local_rotation(const math::Vector& rv);

			Transform& apply(const math::TransformVectors& tform);

			math::Vector get_local_direction_vector(const math::Vector forward = { 0.0f, 0.0f, -1.0f }); // 1.0f

			Transform& set_rx(float rx);
			Transform& set_ry(float ry);
			Transform& set_rz(float rz);

			Transform& move(const math::Vector& tv, bool local=false);

			// Orients this transform to look at 'target', then returns the new basis.
			math::RotationMatrix look_at(const math::Vector& target, const math::Vector& up={0.0f, 1.0f, 0.0f});

			// Orients this transform to look at the 't' Transform's position, then returns the new basis.
			math::RotationMatrix look_at(Transform& t, const math::Vector& up={0.0f, 1.0f, 0.0f});

			// Rotation:
			Transform& rotate(const math::Vector& rv, bool local=false);

			Transform& rotateX(float rx, bool local=false);
			Transform& rotateY(float ry, bool local=false);
			Transform& rotateZ(float rz, bool local=false);

			// Local transformations:
			inline math::Vector         get_local_position() const { return transform.translation; } // math::get_translation(transform._m);
			inline math::Vector         get_local_scale()    const { return transform.scale; }
			inline math::RotationMatrix get_local_basis()    const { return transform.basis; }

			// Returns a copy of the 'model' matrix.
			math::Matrix get_local_matrix();
			math::Matrix get_inverse_local_matrix();

			Transform& set_matrix(const math::Matrix& m);

			Transform& set_local_matrix(const math::Matrix& m);
			Transform& set_local_position(const math::Vector& position);
			Transform& set_local_scale(const math::Vector& scale);

			Transform& set_local_basis(const math::RotationMatrix& basis);
			Transform& set_local_basis_q(const math::Quaternion& basis);
	};
}