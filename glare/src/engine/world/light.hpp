#pragma once

#include <engine/world/entity.hpp>
#include <engine/world/camera.hpp>
#include <graphics/types.hpp>

namespace graphics
{
	class Shader;
}

namespace engine
{
	struct LightComponent
	{
		static LightType resolve_light_mode(const std::string& mode);

		LightType type = LightType::Point;
		graphics::ColorRGB color = { 1.0f, 1.0f, 1.0f };

		CameraParameters shadow_perspective;

	};

	Entity create_light(World& world, const math::Vector& position, const graphics::ColorRGB& color, LightType type=LightType::Point, Entity parent=null, bool debug_mode=false, pass_ref<graphics::Shader> debug_shader={});
}