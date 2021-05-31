#include "camera.hpp"

#include <math/math.hpp>
#include <engine/world/world.hpp>

#include <util/string.hpp>

namespace engine
{
	CameraParameters::Projection CameraParameters::resolve_projection_mode(const std::string& mode)
	{
		auto m = util::lowercase(mode);

		if (m.starts_with("ortho"))
		{
			return Projection::Orthographic;
		}

		return Projection::Perspective;
	}

	CameraParameters::CameraParameters(const util::json& camera_cfg)
		: CameraParameters
		(
			util::get_value(camera_cfg, "fov", CameraParameters::DEFAULT_FOV),
			util::get_value(camera_cfg, "near", CameraParameters::NEAR_PLANE),
			util::get_value(camera_cfg, "far", CameraParameters::FAR_PLANE),
			util::get_value(camera_cfg, "aspect_ratio", CameraParameters::ASPECT),

			resolve_projection_mode(util::get_value<std::string>(camera_cfg, "projection", "perspective")),

			util::get_value(camera_cfg, "free_rotation", CameraParameters::DEFAULT_FREE_ROTATION),
			util::get_value(camera_cfg, "dynamic_aspect_ratio", CameraParameters::DEFAULT_DYNAMIC_ASPECT_RATIO)
		) {}

	CameraParameters::CameraParameters(float v_fov_deg, float near_plane, float far_plane, float aspect_ratio, CameraParameters::Projection projection_mode, bool free_rotation, bool dynamic_aspect_ratio)
		: fov(glm::radians(v_fov_deg)), near_plane(near_plane), far_plane(far_plane), aspect_ratio(aspect_ratio), projection_mode(projection_mode), free_rotation(free_rotation), dynamic_aspect_ratio(dynamic_aspect_ratio){}

	Entity create_camera(World& world, CameraParameters params, Entity parent, bool make_active)
	{
		auto& registry = world.get_registry();
		
		auto entity = create_entity(world, parent, EntityType::Camera);

		registry.emplace<CameraParameters>(entity, params);

		world.add_camera(entity, make_active);

		return entity;
	}
}