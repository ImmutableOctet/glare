#pragma once

#include <engine/world/entity.hpp>
#include <engine/world/camera.hpp>
//#include <graphics/types.hpp>
#include <graphics/cubemap_transform.hpp>
#include <graphics/shadow_map.hpp>

#include <optional>

namespace graphics
{
	class Context;
	class Shader;
	class Texture;
	class FrameBuffer;
}

namespace engine
{
	struct LightComponent
	{
		static LightType resolve_light_mode(const std::string& mode);

		LightType type = LightType::Point;
		graphics::ColorRGB color = { 1.0f, 1.0f, 1.0f };
	};

	struct PointLightShadows
	{
		using TFormData = graphics::CubeMapTransforms;
		using ShadowMap = graphics::ShadowMap;

		PointLightShadows(pass_ref<graphics::Context> context, const CameraParameters& shadow_perspective, const TFormData& transforms, const math::vec2i& resolution);
		PointLightShadows(PointLightShadows&&) = default;

		PointLightShadows& operator=(PointLightShadows&&) noexcept = default;

		CameraParameters shadow_perspective;
		TFormData transforms;

		ShadowMap shadow_map;
	};

	Entity create_light(World& world, const math::Vector& position, const graphics::ColorRGB& color, LightType type=LightType::Point, Entity parent=null, bool debug_mode=false, pass_ref<graphics::Shader> debug_shader={});

	// Attaches light-point shadows to the 'light' entity specified.
	void attach_shadows(World& world, Entity light, const math::vec2i& resolution);
	void attach_shadows(World& world, Entity light, std::optional<math::vec2i> resolution=std::nullopt, std::optional<CameraParameters> perspective_cfg=std::nullopt, bool update_aspect_ratio=true);
}