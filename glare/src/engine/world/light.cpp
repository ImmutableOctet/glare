#include "light.hpp"
#include <engine/world/world.hpp>

#include <engine/resource_manager.hpp>
#include <engine/world/graphics_entity.hpp>

#include <engine/model_component.hpp>

namespace engine
{
	Entity create_light(World& world, const math::Vector& position, const graphics::ColorRGB& color, LightType type, Entity parent)
	{
		const auto& res = world.get_resource_manager();

		//auto light = create_pivot(world, position, parent, EntityType::Light);
		auto light = load_model(world, "assets/geometry/cube.b3d", parent, EntityType::Geometry, false, 0.0f, std::nullopt, std::nullopt, std::nullopt, res.get_shader("assets/shaders/light_box.vert", "assets/shaders/light_box.frag"));

		auto t = world.set_position(light, position);
		t.set_scale(4.0f);

		auto& registry = world.get_registry();
		auto& model = registry.get<ModelComponent>(light);

		model.color = { color, 1.0f };

		world.get_registry().emplace<LightComponent>(light, type, color);

		return light;
	}
}