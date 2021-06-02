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
	struct LightProperties;

	struct DirectionalLightComponent
	{
		//math::Vector direction = {};
		bool use_position = false;
	};

	struct SpotLightComponent
	{
		static constexpr float DEFAULT_CONSTANT = 1.0f;
		static constexpr float DEFAULT_LINEAR = 0.09f;
		static constexpr float DEFAULT_QUADRATIC = 0.032f;
		inline static float DEFAULT_CUTOFF() { return glm::cos(glm::radians(12.5f)); }; // constexpr
		inline static float DEFAULT_OUTER_CUTOFF() { return glm::cos(glm::radians(17.5f)); }; // constexpr

		float cutoff = DEFAULT_CUTOFF();
		float outer_cutoff = DEFAULT_OUTER_CUTOFF();

		float constant = DEFAULT_CONSTANT;
		float linear = DEFAULT_LINEAR;
		float quadratic = DEFAULT_QUADRATIC;
	};

	struct PointLightComponent
	{
		static constexpr float DEFAULT_LINEAR = 0.007f; // 0.1f;
		static constexpr float DEFAULT_QUADRATIC = 0.0002f; //(1.0f / 2000.0f);

		float linear = DEFAULT_LINEAR;
		float quadratic = DEFAULT_QUADRATIC;

		inline float get_radius(float max_brightness, float constant=1.0f) const
		{
			return (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * max_brightness))) / (2.0f * quadratic);
		}

		float get_radius(const LightProperties& properties, float constant=1.0f) const;
	};

	struct LightProperties
	{
		using Directional = DirectionalLightComponent;
		using Spot = SpotLightComponent;
		using Point = PointLightComponent;

		static float calculate_max_brightness(const graphics::ColorRGB& diffuse);

		graphics::ColorRGB ambient = { 0.1f, 0.1f, 0.1f };
		graphics::ColorRGB diffuse = { 1.0f, 1.0f, 1.0f }; // color;
		graphics::ColorRGB specular = {};

		float max_brightness() const;
	};

	struct LightComponent
	{
		using Directional = DirectionalLightComponent;
		using Spot = SpotLightComponent;
		using Point = PointLightComponent;

		using Properties = LightProperties;

		static LightType resolve_light_mode(const std::string& mode);

		LightType type = LightType::Point;

		Properties properties;
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

	Entity create_directional_light(World& world, const math::Vector& position, const LightProperties& properties={}, const LightProperties::Directional& advanced_properties={}, Entity parent=null, bool debug_mode=false, pass_ref<graphics::Shader> debug_shader={});
	Entity create_spot_light(World& world, const math::Vector& position, const LightProperties& properties = {}, const LightProperties::Spot& advanced_properties={}, Entity parent=null, bool debug_mode=false, pass_ref<graphics::Shader> debug_shader={});
	Entity create_point_light(World& world, const math::Vector& position, const LightProperties& properties={}, const LightProperties::Point& advanced_properties={}, Entity parent=null, bool debug_mode=false, pass_ref<graphics::Shader> debug_shader={});

	// Attaches light-point shadows to the 'light' entity specified.
	void attach_shadows(World& world, Entity light, const math::vec2i& resolution);
	void attach_shadows(World& world, Entity light, LightType light_type, std::optional<math::vec2i> resolution=std::nullopt, std::optional<CameraParameters> perspective_cfg=std::nullopt, bool update_aspect_ratio=true);

	// Update shadow-map state (LightShadows, etc.) based on the type of light specified.
	void update_shadows(World& world, Entity light);
}