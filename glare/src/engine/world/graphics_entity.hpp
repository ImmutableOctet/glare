#pragma once

#include <types.hpp>
#include <graphics/types.hpp>
#include "entity.hpp"

namespace graphics
{
	class Model;
}

namespace engine
{
	class World;

	// Entity with 3D Model component.
	Entity create_model(World& world, pass_ref<graphics::Model> model, Entity parent=null);

	Entity attach_model(World& world, Entity entity, pass_ref<graphics::Model> model, graphics::ColorRGBA color = { 1.0f, 1.0f, 1.0f, 1.0f });
}