#include "transform.hpp"

#include <glm/gtx/matrix_decompose.hpp>

using namespace math;

namespace engine
{
	TransformViewData::TransformViewData(Registry& registry, Entity entity)
		: TransformViewData(registry, entity, registry.get_or_assign<Relationship>(entity)) {}

	TransformViewData::TransformViewData(Registry& registry, Entity entity, const Relationship& relationship)
		: TransformViewData(registry, entity, relationship, registry.get<TransformComponent>(entity)) {}

	TransformViewData::TransformViewData(Registry& registry, Entity entity, const Relationship& relationship, TransformComponent& transform)
		: registry(registry), relationship(relationship), transform(transform), entity(entity) {}

	std::optional<TransformViewData> TransformViewData::get_parent_data(const TransformViewData& data)
	{
		if (data.relationship.get_parent() != null)
		{
			return TransformViewData(data.registry, data.relationship.get_parent());
		}

		return std::nullopt;
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

	void Transform::recalculate(bool force)
	{
		update_local_matrix(force);
		update_matrix(force);
		update_inverse_matrix(force);
	}

	void Transform::apply_basis(const math::RotationMatrix& basis, bool local)
	{
		if (local)
		{
			auto local_basis = get_local_basis();

			set_local_basis((local_basis * basis));
		}
		else
		{
			auto current_basis = get_basis();

			set_basis((basis * current_basis));
		}
	}

	void Transform::invalidate()
	{
		transform._dirty |= Dirty::M;

		invalidate_world();
	}

	void Transform::invalidate_world()
	{
		if ((transform._dirty & Dirty::W))
		{
			return;
		}

		transform._dirty |= (Dirty::W | Dirty::IW);

		relationship.enumerate_children(registry, [&](auto child, auto& child_relationship, auto next_child)
		{
			auto t = Transform(registry, child, child_relationship);

			t.invalidate_world();

			return true;
		});
	}

	Transform::Transform(TransformViewData data)
		: TransformViewData(data), parent_data(TransformViewData::get_parent_data(data)) {}

	Transform::Transform(Registry& registry, Entity entity)
		: Transform(registry, entity, registry.get_or_assign<Relationship>(entity)) {}

	Transform::Transform(Registry& registry, Entity entity, const Relationship& relationship)
		: Transform(registry, entity, relationship, registry.get<TransformComponent>(entity)) {}

	Transform::Transform(Registry& registry, Entity entity, const Relationship& relationship, TransformComponent& transform)
		: TransformViewData({registry, entity, relationship, transform }) {}

	Transform::~Transform()
	{
		recalculate(false);
	}

	math::Matrix Transform::get_matrix()
	{
		return update_matrix();
	}

	math::Matrix Transform::get_inverse_matrix()
	{
		return update_inverse_matrix();
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

	void Transform::set_position(const math::Vector& position)
	{
		auto parent = get_parent();

		set_local_position((parent) ? (parent->get_inverse_matrix() * position) : position);

		invalidate();
	}

	void Transform::set_scale(const math::Vector& scale)
	{
		auto parent = get_parent();

		set_local_scale((parent) ? (scale / parent->get_scale()) : scale);

		invalidate();
	}

	void Transform::set_basis(const math::RotationMatrix& basis)
	{
		auto parent = get_parent();

		set_local_basis((parent) ? (parent->get_basis()) : basis );

		invalidate();
	}

	void Transform::rotate(const math::Vector& rv, bool local)
	{
		apply_basis(math::rotation_from_vector(rv), local);
	}

	void Transform::rotateX(float rx, bool local)
	{
		apply_basis(math::rotation_pitch(rx), local);
	}

	void Transform::rotateY(float ry, bool local)
	{
		apply_basis(math::rotation_yaw(ry), local);
	}

	void Transform::rotateZ(float rz, bool local)
	{
		apply_basis(math::rotation_roll(rz), local);
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

	void Transform::set_local_matrix(const math::Matrix& m)
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
		set_local_basis(math::to_rotation_matrix(glm::conjugate(rotation))); // <-- Review behavior with vs. without 'glm::conjugate'.

		invalidate();
	}

	void Transform::set_local_position(const math::Vector& position)
	{
		transform.translation = position;

		invalidate();

		//update_local_matrix(position, get_local_scale(), get_local_basis());
	}

	void Transform::set_local_scale(const math::Vector& scale)
	{
		transform.scale = scale;

		invalidate();

		//update_local_matrix(get_local_position(), scale, get_local_basis());
	}

	void Transform::set_local_basis(const math::RotationMatrix& basis)
	{
		transform.basis = basis;

		invalidate();

		//update_local_matrix(get_local_position(), get_local_scale(), basis);
	}
}