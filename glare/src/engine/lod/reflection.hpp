#pragma once

#include <engine/reflection.hpp>

namespace engine
{
	class EntityBatchSystem;

	template <> void reflect<EntityBatchSystem>();
}