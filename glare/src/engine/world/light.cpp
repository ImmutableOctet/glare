#include "light.hpp"

#include <engine/world/world.hpp>

#include <engine/resource_manager/resource_manager.hpp>
#include <engine/world/graphics_entity.hpp>

#include <engine/model_component.hpp>

#include <util/string.hpp>
#include <util/algorithm.hpp>
#include <util/optional.hpp>

#include <graphics/context.hpp>
//#include <graphics/shader.hpp>
#include <graphics/texture.hpp>
#include <graphics/framebuffer.hpp>
#include <graphics\shadow_map.hpp>

#include <cmath>

namespace engine
{
	float PointLightComponent::get_radius(const LightProperties& properties, float constant) const
	{
		return get_radius(properties.max_brightness(), constant);
	}

	float LightProperties::calculate_max_brightness(const graphics::ColorRGB& diffuse)
	{
		return std::fmaxf(std::fmaxf(diffuse.r, diffuse.g), diffuse.b);
	}

	float LightProperties::max_brightness() const
	{
		return calculate_max_brightness(diffuse);
	}

	static Entity create_light_impl(World& world, const math::Vector& position, LightType type, const LightProperties& properties, Entity parent=null, bool debug_mode=false, pass_ref<graphics::Shader> debug_shader={})
	{
		Entity light = null;

		if (debug_mode) // (debug_shader)
		{
			//const auto& res = world.get_resource_manager();

			light = load_model(world, "assets/geometry/cube.b3d", parent, EntityType::Light, false, false, 0.0f, std::nullopt, std::nullopt, std::nullopt, debug_shader); // {}

			auto t = world.set_position(light, position);
			t.set_scale(4.0f);

			auto& registry = world.get_registry();
			auto& model = registry.get<ModelComponent>(light);

			model.color = { properties.diffuse, 1.0f };
			model.casts_shadow = false;
		}
		else
		{
			light = create_pivot(world, position, parent, EntityType::Light);
		}

		world.get_registry().emplace<LightComponent>
		(
			light,

			LightComponent
			{
				.type { type },
				.properties { properties }
			}
		);

		return light;
	}

	Entity create_directional_light(World& world, const math::Vector& position, const LightProperties& properties, const LightProperties::Directional& advanced_properties, Entity parent, bool debug_mode, pass_ref<graphics::Shader> debug_shader)
	{
		auto light = create_light_impl(world, position, LightType::Directional, properties, parent, debug_mode, debug_shader);

		// Currently disabled due to zero members in 'DirectionalLightComponent'.
		world.get_registry().emplace<DirectionalLightComponent>(light, advanced_properties);

		return light;
	}

	Entity create_spot_light(World& world, const math::Vector& position, const LightProperties& properties, const LightProperties::Spot& advanced_properties, Entity parent, bool debug_mode, pass_ref<graphics::Shader> debug_shader)
	{
		auto light = create_light_impl(world, position, LightType::Spotlight, properties, parent, debug_mode, debug_shader);

		world.get_registry().emplace<SpotLightComponent>(light, advanced_properties);

		return light;
	}

	Entity create_point_light(World& world, const math::Vector& position, const LightProperties& properties, const LightProperties::Point& advanced_properties, Entity parent, bool debug_mode, pass_ref<graphics::Shader> debug_shader)
	{
		auto light = create_light_impl(world, position, LightType::Point, properties, parent, debug_mode, debug_shader);

		world.get_registry().emplace<PointLightComponent>(light, advanced_properties);

		return light;
	}

	bool attach_shadows(World& world, Entity light, const math::vec2i& resolution, float shadow_range, std::optional<LightType> shadow_light_type)
	{
		auto& registry = world.get_registry();

		LightType light_type = util::value_or
		(
			shadow_light_type,

			[&registry, &light]()
			{
				const auto& light_cfg = registry.get<LightComponent>(light);

				return light_cfg.type;
			}
		);

		CameraParameters perspective_cfg;
		
		switch (light_type)
		{
			case LightType::Point:
				perspective_cfg = CameraParameters(90.0f, 1.0f, shadow_range, 0.0, CameraProjection::Perspective);

				break;

			case LightType::Directional:
				perspective_cfg = CameraParameters(90.0f, 0.0f, shadow_range, 1.0f, CameraProjection::Orthographic); // 1.0f
				//perspective_cfg = CameraParameters(45.0f, 1.0f, shadow_range, 1.0, CameraProjection::Perspective);

				break;
		}

		return attach_shadows(world, light, light_type, resolution, perspective_cfg, true, true);
	}

	template <typename TFormData>
	static TFormData get_tform_data(const math::Matrix4x4& light_projection, const math::Vector& light_position)
	{
		//static_assert(false, "Implementation missing.");
		assert(false);
	}

	template <>
	static PointLightShadows::TFormData get_tform_data<PointLightShadows::TFormData>(const math::Matrix4x4& light_projection, const math::Vector& light_position)
	{
		return PointLightShadows::TFormData(light_projection, light_position);
	}

	template <>
	static DirectionLightShadows::TFormData get_tform_data<DirectionLightShadows::TFormData>(const math::Matrix4x4& light_projection, const math::Vector& light_position)
	{
		auto view = glm::lookAt(light_position, math::Vector(0.0f), math::Vector(0.0f, 1.0f, 0.0f));
		auto light_space = light_projection * view;

		return light_space;
	}

	template <LightType light_type, typename ShadowType, typename TFormData>
	static bool attach_shadows_impl(World& world, Entity light, const math::Vector& light_position, std::optional<math::vec2i> resolution, std::optional<CameraParameters> perspective_cfg, bool update_aspect_ratio)
	{
		auto& registry = world.get_registry();

		auto* shadows = registry.try_get<ShadowType>(light);

		auto* pcfg_ptr = util::get_mutable(perspective_cfg, (shadows) ? &shadows->shadow_perspective : nullptr);

		// Either the caller must specify perspective parameters, or the
		// light-entity must already have the 'PointLightShadows' component.
		//assert(pcfg_ptr); // <-- This should only fail if you're trying to attach shadows to a light without any parameters.
		if (!pcfg_ptr)
		{
			return false;
		}

		auto& pcfg = *pcfg_ptr;

		if ((update_aspect_ratio) && (resolution.has_value()))
		{
			pcfg.aspect_ratio = CameraParameters::calculate_aspect_ratio(*resolution);
		}

		math::Matrix4x4 light_projection;

		switch (pcfg.projection_mode)
		{
			case CameraProjection::Orthographic:
			{
				math::vec2i r;

				if (resolution)
				{
					r = *resolution;
				}
				else
				{
					assert(shadows);

					r = shadows->shadow_map.get_resolution();
				}

				auto div = 16; // 24;

				auto width = r.x/div; // 20.0f; // 200.0f;
				auto height = r.y/div; // 20.0f; // 200.0f;

				//auto width = std::sqrtf(r.x);
				//auto height = std::sqrtf(r.y);

				auto hw = (width / 2.0f);
				auto hh = (height / 2.0f);

				light_projection = glm::ortho(-hw, hw, -hh, hh, pcfg.near_plane, pcfg.far_plane);
				//light_projection = glm::ortho(-2048.0f, 2048.0f, -2048.0f, 2048.0f, 1.0f, 4000.0f);

				break;
			}
			case CameraProjection::Perspective:
				light_projection = glm::perspective(pcfg.fov, pcfg.aspect_ratio, pcfg.near_plane, pcfg.far_plane);

				break;
			/*
			default:
				assert(false);

				break;
			*/
		}

		auto tform = get_tform_data<TFormData>(light_projection, light_position);

		if ((shadows) && util::equal_or_nullopt(resolution, shadows->shadow_map.get_resolution()))
		{
			// No need to alter the shadow-map portion, simply update the perspective-config and cubemap transform:
			if (perspective_cfg.has_value())
			{
				shadows->shadow_perspective = *perspective_cfg; // 'pcfg' is not used because the underlying object could be the same.
			}

			shadows->transforms = tform;
		}
		else
		{
			registry.emplace_or_replace<ShadowType>(light, world.get_resource_manager().get_context(), pcfg, tform, *resolution);
		}

		return true;
	}
	
	bool attach_shadows(World& world, Entity light, LightType light_type, std::optional<math::vec2i> resolution, std::optional<CameraParameters> perspective_cfg, bool update_aspect_ratio, bool conform_to_light_type)
	{
		auto& registry = world.get_registry();

		auto light_tform = world.get_transform(light);
		auto light_position = light_tform.get_position();

		if (conform_to_light_type)
		{
			switch (light_type)
			{
				case LightType::Point:
					return attach_shadows_impl<LightType::Point, PointLightShadows, PointLightShadows::TFormData>(world, light, light_position, resolution, perspective_cfg, update_aspect_ratio);

					//break;
				case LightType::Directional:
					return attach_shadows_impl<LightType::Directional, DirectionLightShadows, DirectionLightShadows::TFormData>(world, light, light_position, resolution, perspective_cfg, update_aspect_ratio);

					//break;
				//default:
					// Other light types are not currently supported. (Spotlights, etc.)
					//assert(false);
					//break;
			}
		}
		else
		{
			if (attach_shadows_impl<LightType::Point, PointLightShadows, PointLightShadows::TFormData>(world, light, light_position, resolution, perspective_cfg, update_aspect_ratio)) return true;
			if (attach_shadows_impl<LightType::Directional, DirectionLightShadows, DirectionLightShadows::TFormData>(world, light, light_position, resolution, perspective_cfg, update_aspect_ratio)) return true;
		}

		return false;
	}

	void update_shadows(World& world, Entity light)
	{
		auto& registry = world.get_registry();

		const auto* light_component = registry.try_get<LightComponent>(light);

		if (light_component)
		{
			auto light_type = light_component->type;

			attach_shadows(world, light, light_type);
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
}