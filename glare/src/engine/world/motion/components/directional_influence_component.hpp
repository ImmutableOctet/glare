#pragma once

#include <util/sampler.hpp>

namespace engine
{
	struct DirectionalInfluenceComponent
	{
		util::Sampler1D ground;
		util::Sampler1D air;
	};
}