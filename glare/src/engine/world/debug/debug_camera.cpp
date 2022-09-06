#include "debug_camera.hpp"

// Debug Camera behaviors:
#include <engine/world/behaviors/free_look_behavior.hpp>
#include <engine/world/behaviors/debug_move_behavior.hpp>

#include <engine/world/world.hpp>
#include <engine/world/camera.hpp>

namespace engine
{
	Entity create_debug_camera(World& world, float v_fov_deg, Entity parent)
	{
		auto camera = create_camera(world, CameraParameters(v_fov_deg, 0.1f, 10000.0f), parent);

		return attach_debug_camera_features(world, camera);
	}

	Entity attach_debug_camera_features(World& world, Entity camera)
	{
		auto& registry = world.get_registry();

		registry.emplace<FreeLookBehavior>(camera);
		registry.emplace<DebugMoveBehavior>(camera);

		return camera;
	}
}