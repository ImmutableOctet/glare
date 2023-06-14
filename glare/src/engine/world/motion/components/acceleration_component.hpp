#pragma once

#include <util/sampler.hpp>

namespace engine
{
	struct AccelerationComponent
	{
		util::Sampler1D ground;
		util::Sampler1D air;
	};
}