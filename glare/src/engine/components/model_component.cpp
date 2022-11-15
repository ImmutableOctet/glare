#include "model_component.hpp"

#include <graphics/material.hpp>

namespace engine
{
	bool ModelComponent::transparent() const
	{
		if (always_transparent)
		{
			return true;
		}

		return graphics::Material::value_is_transparent(color);
	}
}