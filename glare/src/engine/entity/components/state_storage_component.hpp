#pragma once

#include <engine/entity/state_storage_manager.hpp>

namespace engine
{
	// Used to encapsulate component instances stored local to a state.
	struct StateStorageComponent : public StateStorageManager {};
}