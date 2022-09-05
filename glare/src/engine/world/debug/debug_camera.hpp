#pragma once

//#include <engine/world/entity.hpp>
//#include <engine/world/camera.hpp>

#include <engine/world/camera.hpp>
#include <engine/types.hpp>

namespace engine
{
	Entity create_debug_camera(World& world, float v_fov_deg=CameraParameters::DEFAULT_FOV, Entity parent=null);
	Entity attach_debug_camera_features(World& world, Entity camera);
}