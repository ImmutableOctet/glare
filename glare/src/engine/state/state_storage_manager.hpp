#pragma once

#include <engine/meta/component_storage.hpp>

namespace engine
{
	struct StateStorageManager
	{
		using StateStorage = ComponentStorage;

		util::small_vector<StateStorage, 4> states;

		StateStorage& get_storage(EntityStateIndex index);
	};
}