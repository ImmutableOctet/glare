#include "light.hpp"

#include <engine/world/world.hpp>

#include <engine/resource_manager.hpp>
#include <engine/world/graphics_entity.hpp>

#include <engine/model_component.hpp>

#include <util/string.hpp>

#include <graphics/context.hpp>
//#include <graphics/shader.hpp>
#include <graphics/texture.hpp>
#include <graphics/framebuffer.hpp>

namespace engine
{
	Entity create_light(World& world, const math::Vector& position, const graphics::ColorRGB& color, LightType type, Entity parent, bool debug_mode, pass_ref<graphics::Shader> debug_shader)
	{
		const auto& res = world.get_resource_manager();

		Entity light = null;

		if (debug_mode) // (debug_shader)
		{
			light = load_model(world, "assets/geometry/cube.b3d", parent, EntityType::Light, false, 0.0f, std::nullopt, std::nullopt, std::nullopt, debug_shader); // {}

			auto t = world.set_position(light, position);
			t.set_scale(4.0f);

			auto& registry = world.get_registry();
			auto& model = registry.get<ModelComponent>(light);

			model.color = { color, 1.0f };
		}
		else
		{
			light = create_pivot(world, position, parent, EntityType::Light);
		}

		world.get_registry().emplace<LightComponent>(light, type, color);

		return light;
	}

	LightType LightComponent::resolve_light_mode(const std::string& mode)
	{
		auto m = util::lowercase(mode);

		if (m.starts_with("direction"))
		{
			return LightType::Directional;
		}

		if (m.starts_with("spot"))
		{
			return LightType::Spotlight;
		}

		/*
		if (m.starts_with("point"))
		{
			return LightType::Point;
		}
		*/

		return LightType::Point;
	}

	ShadowMap::ShadowMap(pass_ref<graphics::Context>& context, int width, int height) :
		depth_map
		(
			std::make_shared<graphics::Texture>
			(
				context,
				width, height,
				graphics::TextureFormat::Depth, graphics::ElementType::Float,
				graphics::TextureFlags::Clamp, graphics::TextureType::CubeMap,
				graphics::ColorRGBA { 1.0, 1.0, 1.0, 1.0 }
			)
		),

		framebuffer(std::make_shared<graphics::FrameBuffer>(context))
	{
		context->use(*framebuffer, [&, this]()
		{
			framebuffer->attach(*depth_map);
			//framebuffer.attach(graphics::RenderBufferType::Depth, width, height);

			framebuffer->link();
		});
	}
}