#include "camera.hpp"

#include <math/math.hpp>
#include <engine/world/world.hpp>

namespace engine
{
	CameraParameters::CameraParameters(float v_fov_deg, float near_plane, float far_plane, float aspect_ratio)
		: fov(glm::radians(v_fov_deg)), near_plane(near_plane), far_plane(far_plane), aspect_ratio(aspect_ratio) {}

	Entity create_camera(World& world, CameraParameters params, Entity parent, bool make_active)
	{
		auto& registry = world.get_registry();
		
		auto entity = create_entity(world, parent);

		registry.emplace<CameraParameters>(entity, params);

		world.add_camera(entity, make_active);

		return entity;
	}
}