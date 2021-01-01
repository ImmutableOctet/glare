#include "graphics_entity.hpp"
#include "world.hpp"

#include <engine/components/graphics/graphics.hpp>
#include <graphics/model.hpp>

namespace engine
{
	Entity create_model(World& world, pass_ref<graphics::Model> model, Entity parent)
	{
		auto entity = create_entity(world, parent);

		return attach_model(world, entity, model);
	}

	Entity attach_model(World& world, Entity entity, pass_ref<graphics::Model> model, graphics::ColorRGBA color)
	{
		///ASSERT(model);

		auto& registry = world.get_registry();

		registry.assign<ModelComponent>(entity, model, color);

		return entity;
	}
}