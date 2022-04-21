#include "debug_camera.hpp"
#include "debug.hpp"

#include <engine/free_look.hpp>
#include <engine/world/world.hpp>
#include <engine/world/camera.hpp>

namespace engine
{
	namespace debug
	{
		Entity create_debug_camera(World& world, float v_fov_deg, Entity parent)
		{
			auto camera = create_camera(world, CameraParameters(v_fov_deg, 0.1f, 10000.0f), parent);

			return attach_debug_camera_features(world, camera);
		}

		Entity attach_debug_camera_features(World& world, Entity camera)
		{
			auto& registry = world.get_registry();

			registry.emplace<FreeLook>(camera);
			registry.emplace<DebugMove>(camera);

			return camera;
		}
	}
}