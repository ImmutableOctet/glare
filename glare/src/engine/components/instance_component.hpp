#pragma once

//#include "types.hpp"

#include <string>
//#include <filesystem>

namespace engine
{
	// Indicates the path/identifier of the factory that
	// created or otherwise initialized this entity.
	struct InstanceComponent
	{
		std::string instance; // std::filesystem::path
	};
}