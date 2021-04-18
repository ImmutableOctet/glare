#include "light.hpp"

#include <engine/world/world.hpp>

#include <engine/resource_manager.hpp>
#include <engine/world/graphics_entity.hpp>

#include <engine/model_component.hpp>

#include <util/string.hpp>
#include <util/algorithm.hpp>

#include <graphics/context.hpp>
//#include <graphics/shader.hpp>
#include <graphics/texture.hpp>
#include <graphics/framebuffer.hpp>
#include <graphics\shadow_map.hpp>

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

	void attach_shadows(World& world, Entity light, const math::vec2i& resolution)
	{
		CameraParameters perspective_cfg = CameraParameters(90.0f, 1.0f, 300.0f, 0.0, CameraProjection::Perspective);

		attach_shadows(world, light, resolution, perspective_cfg);
	}
	
	void attach_shadows(World& world, Entity light, std::optional<math::vec2i> resolution, std::optional<CameraParameters> perspective_cfg, bool update_aspect_ratio)
	{
		auto& registry = world.get_registry();

		auto light_tform = world.get_transform(light);
		auto light_position = light_tform.get_position();

		auto* shadows = registry.try_get<PointLightShadows>(light);

		math::Matrix4x4 light_projection;

		auto* pcfg_ptr = util::get_mutable(perspective_cfg, (shadows) ? &shadows->shadow_perspective : nullptr);

		// Either the caller must specify perspective parameters, or the
		// light-entity must already have the 'PointLightShadows' component.
		ASSERT(pcfg_ptr); // <-- This should only fail if you're trying to attach shadows to a light without any parameters.

		auto& pcfg = *pcfg_ptr;

		if ((update_aspect_ratio) && (resolution.has_value()))
		{
			pcfg.aspect_ratio = CameraParameters::calculate_aspect_ratio(*resolution);
		}

		const auto& light_cfg = registry.get<LightComponent>(light);

		switch (light_cfg.type)
		{
			case LightType::Point:
				light_projection = glm::perspective(pcfg.fov, pcfg.aspect_ratio, pcfg.near_plane, pcfg.far_plane);

				break;
			default:
				ASSERT(false);
		}

		auto cubemap_tform = PointLightShadows::TFormData(light_projection, light_position);

		if ((shadows) && util::equal_or_nullopt(resolution, shadows->shadow_map.get_resolution()))
		{
			// No need to alter the shadow-map portion, simply update the perspective-config and cubemap transform:
			if (perspective_cfg.has_value())
			{
				shadows->shadow_perspective = *perspective_cfg; // 'pcfg' is not used because the underlying object could be the same.
			}

			shadows->transforms = cubemap_tform;
		}
		else
		{
			registry.emplace_or_replace<PointLightShadows>(light, world.get_resource_manager().get_context(), pcfg, cubemap_tform, *resolution);
		}
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
	
	PointLightShadows::PointLightShadows(pass_ref<graphics::Context> context, const CameraParameters& shadow_perspective, const TFormData& transforms, const math::vec2i& resolution)
		: shadow_perspective(shadow_perspective), transforms(transforms), shadow_map(context, resolution.x, resolution.y)
	{
	}
}