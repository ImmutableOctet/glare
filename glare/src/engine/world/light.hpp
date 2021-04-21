#pragma once

#include <engine/world/entity.hpp>
#include <engine/world/camera.hpp>
//#include <graphics/types.hpp>
#include <graphics/cubemap_transform.hpp>
#include <graphics/shadow_map.hpp>

#include <graphics/types.hpp>

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

	template <typename TFormType, graphics::TextureType TextureRep>
	struct LightShadows
	{
		using TFormData = TFormType;
		using ShadowMap = graphics::ShadowMap;

		static constexpr graphics::TextureType MapType = TextureRep;

		inline LightShadows(pass_ref<graphics::Context> context, const CameraParameters& shadow_perspective, const TFormData& transforms, const math::vec2i& resolution)
			: shadow_perspective(shadow_perspective), transforms(transforms), shadow_map(context, resolution.x, resolution.y, MapType) {}

		LightShadows(LightShadows&&) = default;

		LightShadows& operator=(LightShadows&&) noexcept = default;

		CameraParameters shadow_perspective;
		TFormData transforms;

		ShadowMap shadow_map;
	};

	using PointLightShadows = LightShadows<graphics::CubeMapTransforms, graphics::TextureType::CubeMap>;
	using DirectionLightShadows = LightShadows<math::Matrix4x4, graphics::TextureType::Texture2D>;

	Entity create_light(World& world, const math::Vector& position, const graphics::ColorRGB& color, LightType type=LightType::Point, Entity parent=null, bool debug_mode=false, pass_ref<graphics::Shader> debug_shader={});

	// Attaches light-point shadows to the 'light' entity specified.
	void attach_shadows(World& world, Entity light, const math::vec2i& resolution);
	void attach_shadows(World& world, Entity light, LightType light_type, std::optional<math::vec2i> resolution=std::nullopt, std::optional<CameraParameters> perspective_cfg=std::nullopt, bool update_aspect_ratio=true);

	// Update shadow-map state (LightShadows, etc.) based on the type of light specified.
	void update_shadows(World& world, Entity light);
}