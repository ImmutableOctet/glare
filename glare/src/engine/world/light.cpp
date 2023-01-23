#include "light.hpp"

#include "components/light_component.hpp"

#include "components/directional_light_component.hpp"
#include "components/directional_light_shadow_component.hpp"
#include "components/point_light_component.hpp"
#include "components/point_light_shadow_component.hpp"
#include "components/spot_light_component.hpp"
//#include "components/spot_light_shadow_component.hpp"

#include <engine/world/world.hpp>

#include <engine/resource_manager/resource_manager.hpp>
#include <engine/world/graphics_entity.hpp>

#include <engine/components/model_component.hpp>

#include <util/string.hpp>
#include <util/algorithm.hpp>
#include <util/optional.hpp>

#include <graphics/context.hpp>
//#include <graphics/shader.hpp>
#include <graphics/texture.hpp>
#include <graphics/framebuffer.hpp>
#include <graphics/shadow_map.hpp>

#include <cmath>

namespace engine
{
	template <typename TFormData>
	static TFormData get_tform_data(const math::Matrix4x4& light_projection, const math::Vector& light_position)
	{
		//static_assert(false, "Implementation missing.");
		assert(false);
	}

	template <>
	static PointLightShadowComponent::TFormData get_tform_data<PointLightShadowComponent::TFormData>(const math::Matrix4x4& light_projection, const math::Vector& light_position)
	{
		return PointLightShadowComponent::TFormData(light_projection, light_position);
	}

	template <>
	static DirectionalLightShadowComponent::TFormData get_tform_data<DirectionalLightShadowComponent::TFormData>(const math::Matrix4x4& light_projection, const math::Vector& light_position)
	{
		auto view = glm::lookAt(light_position, math::Vector(0.0f), math::Vector(0.0f, 1.0f, 0.0f));
		auto light_space = light_projection * view;

		return light_space;
	}

	static std::optional<math::Matrix4x4> generate_light_projection_matrix(const CameraComponent& pov, const math::vec2i& resolution)
	{
		switch (pov.projection_mode)
		{
			case CameraProjection::Orthographic:
			{
				const auto div = 16; // 24;

				const auto width = (resolution.x / div);
				const auto height = (resolution.y / div);

				//auto width = std::sqrtf(resolution.x);
				//auto height = std::sqrtf(resolution.y);

				const auto hw = (width / 2.0f);
				const auto hh = (height / 2.0f);

				return glm::ortho(-hw, hw, -hh, hh, pov.near_plane, pov.far_plane);
			}
			case CameraProjection::Perspective:
			{
				return glm::perspective(pov.fov, pov.aspect_ratio, pov.near_plane, pov.far_plane);
			}
		}

		return {};
	}

	template <LightType light_type, typename ShadowType, typename TFormData>
	static bool attach_shadows_impl
	(
		World& world,

		Entity light,
		
		const math::Vector& light_position,
		std::optional<math::vec2i> resolution,
		std::optional<CameraComponent> perspective_cfg,
		
		bool update_aspect_ratio
	)
	{
		auto& registry = world.get_registry();

		if (auto* shadows = registry.try_get<ShadowType>(light))
		{
			const auto current_shadow_resolution = shadows->shadow_map.get_resolution();

			if ((!resolution) || (*resolution == current_shadow_resolution))
			{
				if (perspective_cfg)
				{
					shadows->shadow_perspective = *perspective_cfg;
				}

				const auto light_projection = generate_light_projection_matrix
				(
					shadows->shadow_perspective,
					resolution.value_or(current_shadow_resolution)
				);

				if (light_projection)
				{
					shadows->transforms = get_tform_data<TFormData>(*light_projection, light_position);
				}

				// Mark this component as patched.
				registry.patch<ShadowType>(light);

				return true; // false;
			}
			else
			{
				resolution = current_shadow_resolution;
			}
		}
		
		if (!resolution)
		{
			return false;
		}
		
		if (!perspective_cfg)
		{
			perspective_cfg = ShadowType::default_shadow_perspective;
		}

		const auto light_projection = generate_light_projection_matrix
		(
			*perspective_cfg,
			*resolution
		);

		if (!light_projection)
		{
			return false;
		}

		const auto light_tform = get_tform_data<TFormData>(*light_projection, light_position);

		auto& shadow_comp = registry.emplace_or_replace<ShadowType>
		(
			light,
			
			world.get_resource_manager().get_context(),
			light_tform,
			*resolution,
			*perspective_cfg
		);

		return true;
	}
	
	bool attach_shadows
	(
		World& world,
		
		Entity light,
		LightType light_type,
		
		std::optional<math::vec2i> resolution_2d,
		std::optional<math::vec2i> cube_map_resolution,
		std::optional<CameraComponent> perspective_cfg,
		
		bool update_aspect_ratio,
		bool conform_to_light_type
	)
	{
		auto& registry = world.get_registry();

		auto light_tform = world.get_transform(light);
		auto light_position = light_tform.get_position();

		if (conform_to_light_type)
		{
			switch (light_type)
			{
				case LightType::Point:
					return attach_shadows_impl<LightType::Point, PointLightShadowComponent, PointLightShadowComponent::TFormData>(world, light, light_position, cube_map_resolution, perspective_cfg, update_aspect_ratio);

				case LightType::Directional:
					return attach_shadows_impl<LightType::Directional, DirectionalLightShadowComponent, DirectionalLightShadowComponent::TFormData>(world, light, light_position, resolution_2d, perspective_cfg, update_aspect_ratio);
			}
		}
		
		if (registry.try_get<PointLightComponent>(light))
		{
			if (!attach_shadows_impl<LightType::Point, PointLightShadowComponent, PointLightShadowComponent::TFormData>(world, light, light_position, cube_map_resolution, perspective_cfg, update_aspect_ratio))
			{
				return false;
			}
		}

		if (registry.try_get<DirectionalLightComponent>(light))
		{
			if (!attach_shadows_impl<LightType::Directional, DirectionalLightShadowComponent, DirectionalLightShadowComponent::TFormData>(world, light, light_position, resolution_2d, perspective_cfg, update_aspect_ratio))
			{
				return false;
			}
		}

		return true;
	}

	void update_shadows(World& world, Entity light)
	{
		auto& registry = world.get_registry();

		const auto* light_component = registry.try_get<LightComponent>(light);

		if (!light_component)
		{
			return;
		}

		if (!light_component->casts_shadows)
		{
			return;
		}
		
 		attach_shadows(world, light, light_component->type);
	}
}