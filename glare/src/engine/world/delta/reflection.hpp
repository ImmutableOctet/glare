#pragma once

#include <engine/reflection.hpp>

namespace engine
{
	class DeltaSystem;

	template <> void reflect<DeltaSystem>();
}