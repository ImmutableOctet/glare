#include "transform.hpp"

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/orthonormalize.hpp>

// Debugging related:
#include <iostream>
#include <vector>
#include <util/log.hpp>

using namespace math;

namespace engine
{
	TransformViewData::TransformViewData(Registry& registry, Entity entity)
		: TransformViewData(registry, entity, registry.get_or_emplace<RelationshipComponent>(entity)) {}

	TransformViewData::TransformViewData(Registry& registry, Entity entity, const RelationshipComponent& relationship)
		: TransformViewData(registry, entity, relationship, registry.get<TransformComponent>(entity)) {}

	TransformViewData::TransformViewData(Registry& registry, Entity entity, const RelationshipComponent& relationship, TransformComponent& transform)
		: registry(registry), relationship(relationship), transform(transform), entity(entity) {}

	std::optional<TransformViewData> TransformViewData::get_parent_data(const TransformViewData& data)
	{
		auto parent = data.relationship.get_parent();

		if (parent != null)
		{
			return TransformViewData(data.registry, parent);
		}

		return std::nullopt;
	}

	std::optional<Transform> Transform::get_transform_safe(Registry& registry, Entity entity)
	{
		bool has_transform = (registry.try_get<TransformComponent>(entity));

		if (has_transform)
		{
			return Transform(registry, entity);
		}

		return std::nullopt;
	}

	math::RotationMatrix Transform::orientation(const math::Vector& origin, const math::Vector& target, const math::Vector& up)
	{
		auto k = glm::normalize(origin - target);
		auto i = glm::normalize(glm::cross(up, k));
		auto j = glm::cross(k, i);

		return RotationMatrix(i, j, k);
	}

	math::Quaternion Transform::quat_orientation(const math::Vector& origin, const math::Vector& target, const math::Vector& up)
	{
		return glm::quatLookAt((origin - target), up);
	}

	std::optional<Transform> Transform::get_parent() const
	{
		if (!parent_data)
		{
			return std::nullopt;
		}

		//auto parent = relationship.get_parent();
		//return Transform(registry, parent);

		return Transform(*parent_data);
	}

	bool Transform::invalid(Dirty flag) const
	{
		return (transform._dirty & flag);
	}

	Transform::Dirty Transform::validate(Dirty flag) const
	{
		transform._dirty &= (~flag);

		return transform._dirty;
	}

	const Transform& Transform::recalculate(bool force) const
	{
		update_local_matrix(force);
		update_matrix(force);
		update_inverse_matrix(force);

		return *this;
	}

	Transform& Transform::apply_basis(const math::RotationMatrix& basis, bool local)
	{
		if (local)
		{
			auto local_basis = get_local_basis();

			set_local_basis((basis * local_basis));
		}
		else
		{
			auto current_basis = get_basis();

			set_basis((current_basis * basis));
		}

		return *this;
	}

	const Transform& Transform::invalidate() const
	{
		transform._dirty |= (Dirty::M | Dirty::EventFlag);

		invalidate_world();

		return *this;
	}

	const Transform& Transform::invalidate_world() const
	{
		if ((transform._dirty & Dirty::W))
		{
			return *this;
		}

		transform._dirty |= (Dirty::W | Dirty::IW);

		//return *this;

		/*
		std::vector<unsigned int> children;
		relationship.get_children(registry, children);

		print("Children: [{}]", fmt::join(children, ", "));
		*/

		///*
		// Recursive `Transform` version:
		relationship.enumerate_children(registry, [&](auto child, auto& child_relationship, auto next_child)
		{
			//print("Invalidating world matrix - I am: {}", child);

			auto t = Transform(registry, child, child_relationship);

			t.invalidate_world();

			return true;
		});
		//*/

		// Manual recursive child enumeration version:
		/*
		relationship.enumerate_children(registry, [&](auto child, auto& child_relationship, auto next_child)
		{
			auto* tform = registry.try_get<TransformComponent>(child);

			tform->_dirty |= (Dirty::W | Dirty::IW);

			return true;
		}, true);
		*/

		return *this;
	}

	Transform::Transform(TransformViewData data)
		: TransformViewData(data), parent_data(TransformViewData::get_parent_data(data)) {}

	Transform::Transform(Registry& registry, Entity entity)
		: Transform(registry, entity, registry.get_or_emplace<RelationshipComponent>(entity)) {}

	Transform::Transform(Registry& registry, Entity entity, const RelationshipComponent& relationship)
		: Transform(registry, entity, relationship, registry.get<TransformComponent>(entity)) {}

	Transform::Transform(Registry& registry, Entity entity, const RelationshipComponent& relationship, TransformComponent& transform)
		: Transform(TransformViewData({registry, entity, relationship, transform })) {}

	Transform::Transform(Registry& registry, Entity entity, TransformComponent& transform)
		: Transform(TransformViewData({registry, entity, registry.get<RelationshipComponent>(entity), transform })) {}

	Transform::~Transform()
	{
		// TODO: Look into optimization.
		// (Removing this line gives a minor performance improvement)
		recalculate(false);
	}

	bool Transform::collision_invalid() const
	{
		bool collision_invalid = invalid(Dirty::EventFlag);

		if (collision_invalid)
		{
			return true;
		}

		auto parent = get_parent();

		if (parent)
		{
			collision_invalid = (collision_invalid || parent->collision_invalid());
		}

		return collision_invalid;
	}

	const Transform& Transform::validate_collision() const
	{
		validate_collision_shallow();

		auto parent = get_parent();

		if (parent)
		{
			parent->validate_collision();
		}

		return *this;
	}

	const Transform& Transform::validate_collision_shallow() const
	{
		//validate(Dirty::EventFlag);

		transform.validate(Dirty::EventFlag);

		return *this;
	}

	const math::Matrix& Transform::get_matrix(bool force_refresh) const
	{
		return update_matrix(force_refresh);
	}

	const math::Matrix& Transform::get_inverse_matrix(bool force_refresh) const
	{
		return update_inverse_matrix(force_refresh);
	}

	math::Matrix Transform::get_camera_matrix() const
	{
		constexpr auto world_up = glm::vec3(0.0f, 1.0f, 0.0f);
		constexpr auto world_forward = glm::vec3(0.0f, 0.0f, -1.0f);

		auto position = get_position();

		auto front = glm::normalize(get_basis() * world_forward);
		auto right = glm::normalize(glm::cross(front, world_up));
		auto up = glm::normalize(glm::cross(right, front));

		return glm::lookAt(position, position + front, up);

		//return get_inverse_matrix();
	}

	math::Vector Transform::get_position() const
	{
		return math::get_translation(get_matrix());
	}

	math::Vector Transform::get_scale() const
	{
		auto parent = get_parent();

		if (parent)
		{
			return (transform.scale * parent->get_scale());
		}

		return transform.scale;
	}

	math::RotationMatrix Transform::get_basis() const
	{
		auto parent = get_parent();

		if (parent)
		{
			return (parent->get_basis() * transform.basis);
		}

		return transform.basis;
	}

	math::Quaternion Transform::get_basis_q() const
	{
		return math::to_quaternion(get_basis());
	}

	math::Quaternion Transform::get_local_basis_q() const
	{
		return math::to_quaternion(get_local_basis());
	}

	math::Vector Transform::get_rotation() const
	{
		return math::get_rotation(get_basis());
	}

	math::Vector Transform::get_local_rotation() const
	{
		return math::get_rotation(get_local_basis());
	}

	math::Vector Transform::get_direction_vector(const math::Vector& forward) const
	{
		return (get_basis() * forward);
	}

	void Transform::set_direction_vector(const math::Vector& direction)
	{
		set_basis(math::rotation_from_vector(direction));
	}

	math::TransformVectors Transform::get_vectors() const
	{
		return
		{
			get_position(),
			get_rotation(),
			get_scale()
		};
	}

	math::Vector Transform::get_local_direction_vector(const math::Vector forward) const
	{
		return (get_local_basis() * forward);
	}

	Transform& Transform::set_position(const math::Vector& position)
	{
		auto parent = get_parent();

		if (parent)
		{
			auto parent_inv_matrix = parent->get_inverse_matrix(true);
			auto new_local_position = (parent_inv_matrix * math::Vector4D(position, 1.0f));
			
			set_local_position(math::Vector3D(new_local_position));
		}
		else
		{
			set_local_position(position);
		}

		//set_local_position((parent) ? (parent->get_inverse_matrix() * position) : position);

		//invalidate();

		return *this;
	}

	Transform& Transform::set_scale(const math::Vector& scale)
	{
		auto parent = get_parent();

		set_local_scale((parent) ? (scale / parent->get_scale()) : scale);

		//invalidate();

		return *this;
	}

	Transform& Transform::set_scale(float scale)
	{
		return set_scale({ scale, scale, scale });
	}

	Transform& Transform::set_basis(const math::RotationMatrix& basis)
	{
		auto parent = get_parent();

		if (parent)
		{
			auto parent_basis = parent->get_basis();

			set_local_basis(glm::transpose(parent_basis) * basis);
		}
		else
		{
			set_local_basis(basis);
		}

		//set_local_basis((parent) ? (parent->get_basis()) : basis );

		//invalidate();

		return *this;
	}

	Transform& Transform::set_basis_q(const math::Quaternion& basis)
	{
		//return set_basis(glm::mat3_cast(basis));
		return set_basis(math::to_rotation_matrix((basis))); // TODO: Review use of 'conjugate'. // glm::conjugate
	}

	Transform& Transform::set_rotation(const math::Vector& rv)
	{
		return set_basis(math::rotation_from_vector(rv));
	}

	Transform& Transform::set_local_rotation(const math::Vector& rv)
	{
		return set_local_basis(math::rotation_from_vector(rv));
	}

	Transform& Transform::apply(const math::TransformVectors& tform)
	{
		auto [position, rotation, scale] = tform;

		set_position(position);
		set_rotation(rotation);
		set_scale(scale);

		return *this;
	}

	Transform& Transform::set_rx(float rx)
	{
		auto rotation = get_rotation();

		return set_rotation({rx, rotation.y, rotation.z});
	}

	Transform& Transform::set_ry(float ry)
	{
		auto rotation = get_rotation();

		return set_rotation({ rotation.x, ry, rotation.z });
	}

	Transform& Transform::set_rz(float rz)
	{
		auto rotation = get_rotation();

		return set_rotation({ rotation.x, rotation.y, rz });
	}

	Transform& Transform::set_yaw(float yaw)
	{
		return set_ry(yaw);
	}

	// Computes the `yaw` angle of `direction`, then calls `set_ry`.
	Transform& Transform::set_yaw(const math::Vector& direction)
	{
		return set_ry(math::direction_to_yaw(direction));
	}

	float Transform::get_pitch() const
	{
		return rx();
	}

	float Transform::get_yaw() const
	{
		return ry();
	}

	float Transform::get_roll() const
	{
		return rz();
	}

	math::Vector Transform::align_vector(const math::Vector& v) const
	{
		auto basis = get_local_basis(); // get_basis();

		return (basis * v);
	}

	Transform& Transform::move(const math::Vector& tv, bool local)
	{
		if (local)
		{
			return set_local_position(get_local_position() + align_vector(tv));
		}

		return set_local_position(get_local_position() + tv);
	}

	math::RotationMatrix Transform::look_at(const math::Vector& target, const math::Vector& up)
	{
		auto position = get_position();

		auto m = orientation(position, target, up);

		set_basis(m);

		return m;
	}

	math::RotationMatrix Transform::look_at(Transform& t, const math::Vector& up)
	{
		return look_at(t.get_position(), up);
	}

	math::RotationMatrix Transform::look_at(Entity entity, const math::Vector& up)
	{
		auto entity_tform = Transform(registry, entity);

		return look_at(entity_tform.get_position(), up);
	}

	Transform& Transform::rotate(const math::Vector& rv, bool local)
	{
		return apply_basis(math::rotation_from_vector(rv), local);
	}

	Transform& Transform::rotateX(float rx, bool local)
	{
		return apply_basis(math::rotation_pitch(rx), local);
	}

	Transform& Transform::rotateY(float ry, bool local)
	{
		return apply_basis(math::rotation_yaw(ry), local);
	}

	Transform& Transform::rotateZ(float rz, bool local)
	{
		return apply_basis(math::rotation_roll(rz), local);
	}

	const math::Matrix& Transform::update_local_matrix(const math::Vector& translation, const math::Vector& scale, const math::RotationMatrix& basis) const
	{
		math::Matrix m = math::identity<math::Matrix>();

		m = glm::translate(m, translation);

		m *= math::Matrix(basis);
		//m = glm::rotate(m, basis);

		m = glm::scale(m, scale);

		transform._m = m;

		return transform._m;
	}

	const math::Matrix& Transform::update_local_matrix(bool force) const
	{
		if (invalid(Dirty::M) || (force))
		{
			update_local_matrix(transform.translation, transform.scale, transform.basis);

			validate(Dirty::M);
		}

		return transform._m;
	}

	const math::Matrix& Transform::update_matrix(bool force) const
	{
		if (invalid(Dirty::W) || (force))
		{
			auto parent = get_parent();

			// TODO: Look into whether we should be passing `force` to parent transforms and local as well.
			transform._w = ((parent) ? (parent->get_matrix(force) * get_local_matrix(force)) : get_local_matrix(force));

			validate(Dirty::W);
		}

		return transform._w;
	}

	const math::Matrix& Transform::update_inverse_matrix(bool force) const
	{
		if (invalid(Dirty::IW) || (force))
		{
			// TODO: Determine if we should be passing the `force` parameter to `get_matrix`.
			transform._iw = glm::inverse(get_matrix(force));

			validate(Dirty::IW);
		}

		return transform._iw;
	}

	math::Matrix Transform::get_local_matrix(bool force) const
	{
		return update_local_matrix(force);
	}

	math::Matrix Transform::get_inverse_local_matrix(bool force) const
	{
		return glm::inverse(get_local_matrix(force));
	}

	Transform& Transform::set_matrix(const math::Matrix& m)
	{
		auto parent = get_parent();

		//invalidate(); <-- Already handled by subroutines.

		if (parent)
		{
			auto parent_inverse_matrix = parent->get_inverse_matrix();

			return set_local_matrix(parent_inverse_matrix * m);
		}

		return set_local_matrix(m);
	}

	Transform& Transform::set_local_matrix(const math::Matrix& matrix)
	{
		/*
		// Unused: Alternative implementation.
		glm::vec3 scale;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;

		glm::quat rotation;

		glm::decompose(m, scale, rotation, translation, skew, perspective);

		transform._m = m;

		set_local_position(translation);
		set_local_scale(scale);
		set_local_basis_q(rotation);

		return invalidate();
		*/

		auto scale = math::get_scaling(matrix);

		math::Vector translation = math::get_translation(matrix); // matrix[3]

		set_local_position(translation);

		//set_basis(glm::scale(matrix, { (1.0 / scale.x), (1.0 / scale.y), (1.0 / scale.z) }));
		//set_scale(scale);

		set_local_basis(glm::scale(matrix, { (1.0 / scale.x), (1.0 / scale.y), (1.0 / scale.z) }));
		set_local_scale(scale);

		//invalidate(); // <-- Already handled by local-coordinate setters, used above.

		return *this;
	}

	Transform& Transform::set_local_position(const math::Vector& position)
	{
		transform.translation = position;

		invalidate();

		//update_local_matrix(position, get_local_scale(), get_local_basis());

		return *this;
	}

	Transform& Transform::set_local_scale(const math::Vector& scale)
	{
		transform.scale = scale;

		invalidate();

		//update_local_matrix(get_local_position(), scale, get_local_basis());

		return *this;
	}

	Transform& Transform::set_local_basis(const math::RotationMatrix& basis)
	{
		transform.basis = basis; // glm::orthonormalize(basis);

		invalidate();

		//update_local_matrix(get_local_position(), get_local_scale(), basis);

		return *this;
	}

	Transform& Transform::set_local_basis_q(const math::Quaternion& basis)
	{
		//return set_local_basis(glm::mat3_cast(basis));
		return set_local_basis(math::to_rotation_matrix(glm::conjugate(basis))); // TODO: Review use of 'conjugate'.
	}
}