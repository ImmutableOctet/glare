#pragma once

#include <engine/reflection.hpp>

namespace engine
{
	class ResourceManager;

	template <> void reflect<ResourceManager>();
}