#pragma once

#include <string>

#include "types.hpp"

#include <math/math.hpp>

namespace engine
{
	struct BoneComponent
	{
		BoneID ID;
		std::string name;
		math::Matrix offset;
	};
}