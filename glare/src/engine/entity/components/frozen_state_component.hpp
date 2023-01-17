#pragma once

#include <engine/entity/state_storage_manager.hpp>

namespace engine
{
	// Used to encapsulate component instances that are (temporarily) excluded during a state.
	struct FrozenStateComponent : public StateStorageManager {};
}