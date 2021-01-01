#include "camera.hpp"

#include <math/math.hpp>
#include <engine/world/world.hpp>

namespace engine
{
	CameraParameters::CameraParameters(float v_fov_deg, float near, float far, float aspect_ratio)
		: fov(glm::radians(v_fov_deg)), near(near), far(far), aspect_ratio(aspect_ratio) {}

	Entity create_camera(World& world, CameraParameters params, Entity parent)
	{
		auto& registry = world.get_registry();
		
		auto entity = create_entity(world, parent);

		registry.assign<CameraParameters>(entity, params);

		world.add_camera(entity);

		return entity;
	}
}