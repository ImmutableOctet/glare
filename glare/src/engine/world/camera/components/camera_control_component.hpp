#pragma once

#include <engine/input/analog.hpp>

namespace engine
{
	struct CameraControlComponent
	{
		// The analog used to rotate/look with the camera.
		// (i.e. change orientation)
		Analog look = Analog::Camera;

		// The analog used to zoom the camera.
		// (e.g. change distance from player)
		Analog zoom = Analog::Zoom;

		// The analog used to move the camera.
		// (Primarily used for debugging purposes)
		Analog move = Analog::Movement;
	};
}