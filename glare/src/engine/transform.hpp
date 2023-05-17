#pragma once

#include <optional>
#include <utility>

#include "types.hpp"

// TODO: Look into whether we should forward declare this instead.
#include "components/transform_component.hpp"

#include "components/relationship_component.hpp"

#include <math/math.hpp>

namespace engine
{
	class World;

	// TODO: Look into moving this to its own header and source files.
	struct TransformViewData
	{
		static std::optional<TransformViewData> get_parent_data(const TransformViewData& data);

		TransformViewData(Registry& registry, Entity entity);
		TransformViewData(Registry& registry, Entity entity, const RelationshipComponent& relationship);
		TransformViewData(Registry& registry, Entity entity, const RelationshipComponent& relationship, TransformComponent& transform);

		Registry& registry; // mutable

		// Entity:

		// NOTE: 'relationship' being a local copy has side-effects,
		// including both increased object size and some very rare edge-cases.
		RelationshipComponent relationship; // const RelationshipComponent&

		TransformComponent& transform; // mutable

		Entity entity;
	};

	// Temporary/scope-local interface type used to efficiently modify and inspect `TransformComponent` objects.
	//
	// TODO: Look into managing updates of the underlying `TransformComponent` via some form of flush/update
	// procedure that interacts with the `registry` field, rather than direct access + caching. (May be slower overall)
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

			const Transform& invalidate() const;
			const Transform& invalidate_world() const;

			// Updates the local 'model' matrix.
			const math::Matrix& update_local_matrix(const math::Vector& translation, const math::Vector& scale, const math::RotationMatrix& basis) const;
			const math::Matrix& update_local_matrix(bool force=false) const;

			const math::Matrix& update_matrix(bool force=false) const;
			const math::Matrix& update_inverse_matrix(bool force=false) const;

			bool invalid(Dirty flag) const;
			Dirty validate(Dirty flag) const;

			const Transform& recalculate(bool force) const;

			Transform(TransformViewData data);
		public:
			using Flag = Dirty;

			Transform(Registry& registry, Entity entity);
			Transform(Registry& registry, Entity entity, const RelationshipComponent& relationship);
			Transform(Registry& registry, Entity entity, const RelationshipComponent& relationship, TransformComponent& transform);

			Transform(Registry& registry, Entity entity, TransformComponent& transform);

			~Transform();

			// Alias to helper function in 'TransformComponent'.
			/*
			template <typename Event>
			inline bool on_flag(Flag flag, Event&& event_fn)
			{
				return transform.on_flag(flag, std::forward<Event>(fn));
			}
			*/

			// Resolves 'parent_data' into a proper 'Transform' object, if applicable.
			std::optional<Transform> get_parent() const;

			// TODO: Rename these routines to be more generic.
			// (i.e. collision is not the only use-case of event flags)
			bool collision_invalid() const;
			const Transform& validate_collision() const;
			const Transform& validate_collision_shallow() const;

			// Global transformations:
			const math::Matrix& get_matrix(bool force_refresh=false) const;
			const math::Matrix& get_inverse_matrix(bool force_refresh = false) const;

			// TODO: Rename this to something better.
			math::Matrix get_camera_matrix() const;

			math::Vector         get_position() const;
			math::Vector         get_scale()    const;
			math::RotationMatrix get_basis()    const;
			math::Quaternion     get_basis_q()  const;

			// Retrieves the current rotation in the form of angles. (Pitch, Yaw, Roll)
			math::Vector get_rotation() const;

			// Retrieves the current local rotation in the form of angles. (Pitch, Yaw, Roll)
			math::Vector get_local_rotation() const;

			// Retrieves a normalized direction vector based on the transform's basis.
			math::Vector get_direction_vector(const math::Vector& forward={0.0f, 0.0f, -1.0f}) const; // {0.0f, 0.0f, 1.0f}

			// Sets the basis of this entity to point towards `direction`.
			// 
			// NOTE: This assumes that `direction` is already normalized.
			// 
			// NOTE: Since this is a direction vector and not a quaternion,
			// this does not preserve local orientation -- i.e. `roll`.
			// 
			// See also: `set_basis`, `set_basis_q`
			Transform& set_direction_vector(const math::Vector& direction);

			// Gradually orients this transform (based on `turn_speed`) to look in `direction`.
			// 
			// NOTE: This assumes that `direction` is already normalized.
			Transform& set_direction_vector(const math::Vector& direction, float turn_speed);

			// Retrieves a normalized direction vector with the `y` element set to zero.
			math::Vector get_flat_direction_vector(const math::Vector& forward={ 0.0f, 0.0f, -1.0f }) const; // {0.0f, 0.0f, 1.0f}

			// Sets the basis of this entity to point towards `direction`, ignoring the `y` axis.
			Transform& set_flat_direction_vector(const math::Vector& direction);

			// Gradually orients this transform (based on `turn_speed`) to look in `direction`, whilst ignoring the `y` axis.
			Transform& set_flat_direction_vector(const math::Vector& direction, float turn_speed);

			math::TransformVectors get_vectors() const;

			math::Vector get_local_direction_vector(const math::Vector forward = { 0.0f, 0.0f, -1.0f }) const; // 1.0f

			inline float get_rx() const { return get_rotation().x; }
			inline float get_ry() const { return get_rotation().y; }
			inline float get_rz() const { return get_rotation().z; }

			inline float get_local_rx() const { return get_local_rotation().x; }
			inline float get_local_ry() const { return get_local_rotation().y; }
			inline float get_local_rz() const { return get_local_rotation().z; }

			inline float rx() const { return get_rx(); }
			inline float ry() const { return get_ry(); }
			inline float rz() const { return get_rz(); }

			inline float local_rx() const { return get_local_rx(); }
			inline float local_ry() const { return get_local_ry(); }
			inline float local_rz() const { return get_local_rz(); }

			inline float get_pitch() const { return get_rx(); }
			inline float get_yaw() const { return get_ry(); }
			inline float get_roll() const { return get_rz(); }

			inline float get_local_pitch() const { return get_local_rx(); }
			inline float get_local_yaw() const { return get_local_ry(); }
			inline float get_local_roll() const { return get_local_rz(); }

			Transform& set_position(const math::Vector& position);
			Transform& set_scale(const math::Vector& scale);
			Transform& set_scale(float scale);

			Transform& set_basis(const math::RotationMatrix& basis);
			Transform& set_basis_q(const math::Quaternion& basis);
			
			Transform& apply_basis(const math::RotationMatrix& basis, bool local=false);
			Transform& apply_basis(const math::RotationMatrix& basis, float turn_speed, bool local=false);
			Transform& apply_basis_q(const math::Quaternion& basis, bool local=false);
			Transform& apply_basis_q(const math::Quaternion& basis, float turn_speed, bool local=false);

			// Euler angles. (Pitch, Yaw, Roll)
			Transform& set_rotation(const math::Vector& rv);

			// Euler angles. (Pitch, Yaw, Roll)
			Transform& set_local_rotation(const math::Vector& rv);

			Transform& apply(const math::TransformVectors& tform);

			Transform& set_rx(float rx);
			Transform& set_ry(float ry);
			Transform& set_rz(float rz);

			Transform& set_rx(float rx, float turn_speed);
			Transform& set_ry(float ry, float turn_speed);
			Transform& set_rz(float rz, float turn_speed);

			Transform& set_ry(const math::Vector& direction);
			Transform& set_ry(const math::Vector& direction, float turn_speed);

			Transform& set_local_rx(float rx);
			Transform& set_local_ry(float ry);
			Transform& set_local_rz(float rz);

			Transform& set_local_rx(float rx, float turn_speed);
			Transform& set_local_ry(float ry, float turn_speed);
			Transform& set_local_rz(float rz, float turn_speed);

			Transform& set_local_ry(const math::Vector& direction);
			Transform& set_local_ry(const math::Vector& direction, float turn_speed);

			inline Transform& set_pitch(float pitch) { return set_rx(pitch); }
			inline Transform& set_pitch(float pitch, float turn_speed) { return set_rx(pitch, turn_speed); }

			inline Transform& set_yaw(float yaw) { return set_ry(yaw); }
			inline Transform& set_yaw(float yaw, float turn_speed) { return set_ry(yaw, turn_speed); }
			inline Transform& set_yaw(const math::Vector& direction) { return set_ry(direction); }
			inline Transform& set_yaw(const math::Vector& direction, float turn_speed) { return set_ry(direction, turn_speed); }

			inline Transform& set_roll(float roll) { return set_rz(roll); }
			inline Transform& set_roll(float roll, float turn_speed) { return set_rz(roll, turn_speed); }

			inline Transform& set_local_pitch(float pitch) { return set_local_rx(pitch); }
			inline Transform& set_local_pitch(float pitch, float turn_speed) { return set_local_rx(pitch, turn_speed); }

			inline Transform& set_local_yaw(float yaw) { return set_local_ry(yaw); }
			inline Transform& set_local_yaw(float yaw, float turn_speed) { return set_local_ry(yaw, turn_speed); }
			inline Transform& set_local_yaw(const math::Vector& direction) { return set_local_ry(direction); }
			inline Transform& set_local_yaw(const math::Vector& direction, float turn_speed) { return set_local_ry(direction, turn_speed); }

			inline Transform& set_local_roll(float roll) { return set_local_rz(roll); }
			inline Transform& set_local_roll(float roll, float turn_speed) { return set_local_rz(roll, turn_speed); }

			// Aligns a vector to the direction/basis of this entity.
			// (Useful for things like determining local movement before it happens)
			math::Vector align_vector(const math::Vector& v, bool local=false) const;

			// Moves this entity forward by `tv`.
			// If `local` is true, this applies `align_vector` to `tv` before affecting the local position.
			Transform& move(const math::Vector& tv, bool local=false);

			// Orients this transform to look at 'target_position'.
			Transform& look_at(const math::Vector& target_position, const math::Vector& up={ 0.0f, 1.0f, 0.0f });

			// Orients this transform gradually (based on `turn_speed`) to look at 'target_position'.
			Transform& look_at(const math::Vector& target_position, float turn_speed, const math::Vector& up={0.0f, 1.0f, 0.0f});

			// Orients this transform to look at `target_tform`.
			Transform& look_at(const Transform& target_tform, const math::Vector& up={ 0.0f, 1.0f, 0.0f });

			// Orients this transform gradually (based on `turn_speed`) to look at `target_tform`.
			Transform& look_at(const Transform& target_tform, float turn_speed, const math::Vector& up={ 0.0f, 1.0f, 0.0f });

			// Orients this transform to look at `target`.
			Transform& look_at(Entity target, const math::Vector& up={ 0.0f, 1.0f, 0.0f });

			// Orients this transform gradually (based on `turn_speed`) to look at `target`.
			Transform& look_at(Entity target, float turn_speed, const math::Vector& up={ 0.0f, 1.0f, 0.0f });

			// Rotation:
			Transform& rotate(const math::Vector& rv, bool local=false);

			Transform& rotateX(float rx, bool local=false);
			Transform& rotateY(float ry, bool local=false);
			Transform& rotateZ(float rz, bool local=false);

			// Local transformations:
			inline const math::Vector&         get_local_position() const { return transform.translation; } // math::get_translation(transform._m);
			inline const math::Vector&         get_local_scale()    const { return transform.scale; }
			inline const math::RotationMatrix& get_local_basis()    const { return transform.basis; }

			math::Quaternion get_local_basis_q() const;

			// Returns a copy of the 'model' matrix.
			math::Matrix get_local_matrix(bool force=false) const;
			math::Matrix get_inverse_local_matrix(bool force=false) const;

			Transform& set_matrix(const math::Matrix& m);

			Transform& set_local_matrix(const math::Matrix& m);
			Transform& set_local_position(const math::Vector& position);
			Transform& set_local_scale(const math::Vector& scale);

			Transform& set_local_basis(const math::RotationMatrix& basis);
			Transform& set_local_basis_q(const math::Quaternion& basis);
	};
}