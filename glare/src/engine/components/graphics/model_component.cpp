#include "model_component.hpp"

#include <graphics/material.hpp>

bool engine::ModelComponent::transparent() const
{
	if (always_transparent)
	{
		return true;
	}

	return graphics::Material::value_is_transparent(color);
}