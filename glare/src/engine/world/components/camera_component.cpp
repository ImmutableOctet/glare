#include "camera_component.hpp"

#include <math/math.hpp>
#include <util/string.hpp>

namespace engine
{
	CameraComponent::Projection CameraComponent::resolve_projection_mode(const std::string& mode)
	{
		auto m = util::lowercase(mode);

		if (m.starts_with("ortho"))
		{
			return Projection::Orthographic;
		}

		return Projection::Perspective;
	}

	CameraComponent::CameraComponent(const util::json& camera_cfg)
		: CameraComponent
		(
			util::get_value(camera_cfg, "fov", CameraComponent::DEFAULT_FOV),
			util::get_value(camera_cfg, "near", CameraComponent::NEAR_PLANE),
			util::get_value(camera_cfg, "far", CameraComponent::FAR_PLANE),
			util::get_value(camera_cfg, "aspect_ratio", CameraComponent::ASPECT),

			resolve_projection_mode(util::get_value<std::string>(camera_cfg, "projection", "perspective")),

			util::get_value(camera_cfg, "free_rotation", CameraComponent::DEFAULT_FREE_ROTATION),
			util::get_value(camera_cfg, "dynamic_aspect_ratio", CameraComponent::DEFAULT_DYNAMIC_ASPECT_RATIO)
		) {}

	CameraComponent::CameraComponent(float v_fov_deg, float near_plane, float far_plane, float aspect_ratio, CameraComponent::Projection projection_mode, bool free_rotation, bool dynamic_aspect_ratio)
		: fov(glm::radians(v_fov_deg)), near_plane(near_plane), far_plane(far_plane), aspect_ratio(aspect_ratio), projection_mode(projection_mode), free_rotation(free_rotation), dynamic_aspect_ratio(dynamic_aspect_ratio) {}
}