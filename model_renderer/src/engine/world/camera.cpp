#include "camera.hpp"

#include <math/math.hpp>
#include <engine/world/world.hpp>

namespace engine
{
	CameraParameters::CameraParameters(float v_fov_deg, float aspect_ratio, float near, float far)
		: fov(glm::radians(v_fov_deg)), aspect_ratio(aspect_ratio), near(near), far(far) {}

	Entity create_camera(World& world, float v_fov_deg, Entity parent)
	{
		auto& registry = world.get_registry();
		
		auto entity = create_entity(world, parent);

		registry.assign<CameraParameters>(entity, v_fov_deg);

		return entity;
	}
}