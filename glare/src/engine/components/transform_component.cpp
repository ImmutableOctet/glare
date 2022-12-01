#include "transform_component.hpp"

namespace engine
{
	bool TransformComponent::invalid(TransformComponent::Dirty flag) const
	{
		return (_dirty & flag);
	}

	void TransformComponent::invalidate(TransformComponent::Dirty flag) const
	{
		_dirty |= flag;
	}

	TransformComponent::Dirty TransformComponent::validate(TransformComponent::Dirty flag) const
	{
		_dirty &= (~flag);

		return _dirty;
	}
}