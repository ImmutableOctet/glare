#include "camera.hpp"

#include <math/math.hpp>
#include <engine/world/world.hpp>

#include <util/string.hpp>

#include <engine/resource_manager/resource_manager.hpp>

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

	Entity create_camera(World& world, CameraParameters params, Entity parent, bool make_active, bool collision_enabled)
	{
		auto& registry = world.get_registry();
		
		constexpr auto entity_type = EntityType::Camera;
		auto entity = create_entity(world, parent, entity_type);

		registry.emplace<CameraParameters>(entity, params);

		// Assign a default name for this camera.
		world.set_name(entity, "Camera");

		if (collision_enabled)
		{
			auto& resource_manager = world.get_resource_manager();
			auto collision_data = resource_manager.generate_sphere_collision(0.1f);

			attach_collision(world, entity, collision_data.collision_shape, {entity_type});
		}

		if ((world.get_camera() == null) || make_active)
		{
			world.set_camera(entity);
		}

		return entity;
	}
}