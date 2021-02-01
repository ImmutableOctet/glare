#include "graphics_entity.hpp"
#include "world.hpp"

#include <engine/resource_manager.hpp>
#include <engine/model_component.hpp>
#include <graphics/model.hpp>

namespace graphics
{
	//class Model;
	class Context;
}

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

		registry.emplace_or_replace<ModelComponent>(entity, model, color);
		registry.emplace_or_replace<RenderFlagsComponent>(entity);

		return entity;
	}

	Entity load_model(World& world, const std::string& path, Entity parent)
	{
		auto& resource_manager = world.get_resource_manager();
		pass_ref<graphics::Model> loaded_model = resource_manager.load_model(path);

		return engine::create_model(world, loaded_model, parent);
	}
}