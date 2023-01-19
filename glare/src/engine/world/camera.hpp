#pragma once

#include "components/camera_component.hpp"

//#include <math/math.hpp>

//#include <engine/world/entity.hpp>
#include <engine/types.hpp>

// Forward declaration not used due to templates.
#include <util/json.hpp>

namespace engine
{
	// Generates a standalone camera entity.
	// 
	// The `make_active` parameter allows you to override `world`'s active camera object.
	// 
	// The `collision_enabled` parameter allows you to generate a simple collision primitive for this entity.
	// (TODO: Remove first-class collision support from this interface)
	Entity create_camera(World& world, CameraComponent params={}, Entity parent=null, bool make_active=false, bool collision_enabled=false);
}