#include "transform.hpp"

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/orthonormalize.hpp>

// Debugging related:
#include <iostream>

using namespace math;

namespace engine
{
	TransformViewData::TransformViewData(Registry& registry, Entity entity)
		: TransformViewData(registry, entity, registry.get_or_emplace<Relationship>(entity)) {}

	TransformViewData::TransformViewData(Registry& registry, Entity entity, const Relationship& relationship)
		: TransformViewData(registry, entity, relationship, registry.get<TransformComponent>(entity)) {}

	TransformViewData::TransformViewData(Registry& registry, Entity entity, const Relationship& relationship, TransformComponent& transform)
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
		bool has_transform = (registry.has<TransformComponent>(entity));

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

	Transform::Dirty Transform::validate(Dirty flag)
	{
		transform._dirty &= (~flag);

		return transform._dirty;
	}

	Transform& Transform::recalculate(bool force)
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

	Transform& Transform::invalidate()
	{
		transform._dirty |= (Dirty::M | Dirty::EventFlag);

		invalidate_world();

		return *this;
	}

	Transform& Transform::invalidate_world()
	{
		if ((transform._dirty & Dirty::W))
		{
			return *this;
		}

		transform._dirty |= (Dirty::W | Dirty::IW);

		relationship.enumerate_children(registry, [&](auto child, auto& child_relationship, auto next_child)
		{
			auto t = Transform(registry, child, child_relationship);

			t.invalidate_world();

			return true;
		});

		return *this;
	}

	Transform::Transform(TransformViewData data)
		: TransformViewData(data), parent_data(TransformViewData::get_parent_data(data)) {}

	Transform::Transform(Registry& registry, Entity entity)
		: Transform(registry, entity, registry.get_or_emplace<Relationship>(entity)) {}

	Transform::Transform(Registry& registry, Entity entity, const Relationship& relationship)
		: Transform(registry, entity, relationship, registry.get<TransformComponent>(entity)) {}

	Transform::Transform(Registry& registry, Entity entity, const Relationship& relationship, TransformComponent& transform)
		: Transform(TransformViewData({registry, entity, relationship, transform })) {}

	Transform::~Transform()
	{
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

	Transform& Transform::validate_collision()
	{
		validate_collision_shallow();

		auto parent = get_parent();

		if (parent)
		{
			parent->validate_collision();
		}

		return *this;
	}

	Transform& Transform::validate_collision_shallow()
	{
		//validate(Dirty::EventFlag);

		transform.validate(Dirty::EventFlag);

		return *this;
	}

	math::Matrix Transform::get_matrix(bool force_refresh)
	{
		return update_matrix(force_refresh);
	}

	math::Matrix Transform::get_inverse_matrix(bool force_refresh)
	{
		return update_inverse_matrix(force_refresh);
	}

	math::Matrix Transform::get_camera_matrix()
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

	math::Vector Transform::get_position()
	{
		return math::get_translation(get_matrix());
	}

	math::Vector Transform::get_scale()
	{
		auto parent = get_parent();

		if (parent)
		{
			return (transform.scale * parent->get_scale());
		}

		return transform.scale;
	}

	math::RotationMatrix Transform::get_basis()
	{
		auto parent = get_parent();

		if (parent)
		{
			return (parent->get_basis() * transform.basis);
		}

		return transform.basis;
	}

	math::Vector Transform::get_rotation()
	{
		return math::get_rotation(get_basis());
	}

	math::Vector Transform::get_local_rotation()
	{
		return math::get_rotation(get_local_basis());
	}

	math::Vector Transform::get_direction_vector(const math::Vector forward)
	{
		return (get_basis() * forward);
	}

	math::TransformVectors Transform::get_vectors()
	{
		return
		{
			get_position(),
			get_rotation(),
			get_scale()
		};
	}

	Transform& Transform::set_position(const math::Vector& position)
	{
		auto parent = get_parent();

		set_local_position((parent) ? (parent->get_inverse_matrix() * position) : position);

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

	math::Vector Transform::get_local_direction_vector(const math::Vector forward)
	{
		return (get_local_basis() * forward);
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

	Transform& Transform::move(const math::Vector& tv, bool local)
	{
		// TODO: Review behavior of 'local'.
		if (!local)
		{
			return set_local_position(get_local_position() + tv);
		}

		auto basis = get_local_basis(); //get_basis();

		auto movement = (basis * tv);

		//set_position(get_position() + movement);
		return set_local_position(get_local_position() + movement);
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

	math::Matrix& Transform::update_local_matrix(const math::Vector& translation, const math::Vector& scale, const math::RotationMatrix& basis)
	{
		math::Matrix m = math::identity<math::Matrix>();

		m = glm::translate(m, translation);

		m *= math::Matrix(basis);
		//m = glm::rotate(m, basis);

		m = glm::scale(m, scale);

		transform._m = m;

		return transform._m;
	}

	math::Matrix& Transform::update_local_matrix(bool force)
	{
		if (invalid(Dirty::M) || (force))
		{
			update_local_matrix(transform.translation, transform.scale, transform.basis);

			validate(Dirty::M);
		}

		return transform._m;
	}

	math::Matrix& Transform::update_matrix(bool force)
	{
		if (invalid(Dirty::W) || (force))
		{
			auto parent = get_parent();

			transform._w = ((parent) ? (parent->get_matrix() * get_local_matrix()) : get_local_matrix());

			validate(Dirty::W);
		}

		return transform._w;
	}

	math::Matrix& Transform::update_inverse_matrix(bool force)
	{
		if (invalid(Dirty::IW) || (force))
		{
			transform._iw = glm::inverse(get_matrix());

			validate(Dirty::IW);
		}

		return transform._iw;
	}

	math::Matrix Transform::get_local_matrix()
	{
		return update_local_matrix();
	}

	math::Matrix Transform::get_inverse_local_matrix()
	{
		return glm::inverse(get_local_matrix());
	}

	Transform& Transform::set_matrix(const math::Matrix& m)
	{
		auto& matrix = m; // transform._m;

		auto scale = math::get_scaling(matrix);

		math::Vector translation = math::get_translation(matrix); // matrix[3]

		////std::cout << "Setting translation to: " << translation << '\n';

		set_local_position(translation);

		//std::cout << "Position is now: " << get_position() << '\n';
		////std::cout << "Position is now: " << get_local_position() << '\n';

		//set_basis(glm::scale(matrix, { (1.0 / scale.x), (1.0 / scale.y), (1.0 / scale.z) }));
		//set_scale(scale);
		set_local_basis(glm::scale(matrix, { (1.0 / scale.x), (1.0 / scale.y), (1.0 / scale.z) }));
		set_local_scale(scale);

		//invalidate();

		return *this;
	}

	Transform& Transform::set_local_matrix(const math::Matrix& m)
	{
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


	// TransformComponent:
	bool TransformComponent::invalid(TransformComponent::Dirty flag) const
	{
		return (_dirty & flag);
	}

	void TransformComponent::invalidate(TransformComponent::Dirty flag)
	{
		_dirty |= flag;
	}

	TransformComponent::Dirty TransformComponent::validate(TransformComponent::Dirty flag)
	{
		_dirty &= (~flag);

		return _dirty;
	}
}