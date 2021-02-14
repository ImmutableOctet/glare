#include "graphics_entity.hpp"
#include "world.hpp"

#include <tuple>

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
	Entity create_model(World& world, pass_ref<graphics::Model> model, Entity parent, EntityType type)
	{
		auto entity = create_entity(world, parent, type);

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

	Entity load_model(World& world, const std::string& path, Entity parent, EntityType type, bool collision_enabled, CollisionGroup solid_mask, CollisionGroup interaction_mask, float mass)
	{
		auto& resource_manager = world.get_resource_manager();

		auto model_data = resource_manager.load_model(path, collision_enabled);

		auto& loaded_model = std::get<0>(model_data);
		auto collision_data = std::get<1>(model_data); // auto*

		auto entity = engine::create_model(world, loaded_model, parent, type);

		if (collision_enabled && collision_data)
		{
			attach_collision(world, entity, collision_data->collision_shape, mass, interaction_mask, solid_mask);
		}

		return entity;
	}
}