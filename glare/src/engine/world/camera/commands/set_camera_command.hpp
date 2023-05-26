#pragma once

#include <engine/command.hpp>

namespace engine
{
	// Sets the active (primary) camera to the `target` entity specified.
	struct SetCameraCommand : public Command {};
}