#pragma once

#include "camera_component.hpp"

#include <engine/world/types.hpp>

//#include <graphics/types.hpp>
#include <graphics/cubemap_transform.hpp>
#include <graphics/shadow_map.hpp>

#include <graphics/types.hpp>

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
		bool use_position : 1 = false;

		inline bool get_use_position() const { return use_position; }
		inline void set_use_position(bool value) { use_position = value; }
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
		static constexpr float DEFAULT_LINEAR = 0.001f; // 0.1f;
		static constexpr float DEFAULT_QUADRATIC = 0.000005f; //(1.0f / 2000.0f);

		float linear = DEFAULT_LINEAR;
		float quadratic = DEFAULT_QUADRATIC;

		// Explicit floating-point version of `get_radius`. (Needed to simplify reflection)
		inline float get_radius_f(float max_brightness, float constant=1.0f) const
		{
			return (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * max_brightness))) / (2.0f * quadratic);
		}

		inline float get_radius(float max_brightness, float constant=1.0f) const
		{
			return get_radius_f(max_brightness, constant);
		}

		float get_radius(const LightProperties& properties, float constant=1.0f) const;
	};

	// TODO: Decouple this type from the rest of the light components/their implementations.
	// TODO: Move this type definition into `world`'s `light` submodule or similar.
	struct LightProperties
	{
		using Directional = DirectionalLightComponent;
		using Spot        = SpotLightComponent;
		using Point       = PointLightComponent;

		static float calculate_max_brightness(const graphics::ColorRGB& diffuse);

		graphics::ColorRGB ambient  = { 0.1f, 0.1f, 0.1f };
		graphics::ColorRGB diffuse  = { 1.0f, 1.0f, 1.0f }; // color;
		graphics::ColorRGB specular = { 0.1f, 0.1f, 0.1f };

		// NOTE: Currently defined in `light` submodule.
		float max_brightness() const;
	};

	struct LightComponent
	{
		using Directional = DirectionalLightComponent;
		using Spot        = SpotLightComponent;
		using Point       = PointLightComponent;

		using Properties = LightProperties;

		// TODO: Replace with `std::string_view`.
		// TODO: Look into using `magic_enum` as an alternative to this function in the general case.
		static LightType resolve_light_mode(const std::string& mode);

		LightType type = LightType::Point;

		Properties properties;
	};

	// TODO: Look into renaming this to something that more clearly indicates that it is a component template.
	template <typename TFormType, graphics::TextureType TextureRep>
	struct LightShadows
	{
		using TFormData = TFormType;
		using ShadowMap = graphics::ShadowMap;

		static constexpr graphics::TextureType MapType = TextureRep;

		inline LightShadows(pass_ref<graphics::Context> context, const CameraParameters& shadow_perspective, const TFormData& transforms, const math::vec2i& resolution)
			: shadow_perspective(shadow_perspective), transforms(transforms), shadow_map(context, resolution.x, resolution.y, MapType) {}

		LightShadows(LightShadows&&) noexcept = default;

		LightShadows& operator=(LightShadows&&) noexcept = default;

		CameraParameters shadow_perspective;
		TFormData transforms;

		ShadowMap shadow_map;
	};

	// TODO: Look into renaming these aliases to better indicate that they are component types:
	using PointLightShadows     = LightShadows<graphics::CubeMapTransforms, graphics::TextureType::CubeMap>;
	using DirectionLightShadows = LightShadows<math::Matrix4x4, graphics::TextureType::Texture2D>;
}