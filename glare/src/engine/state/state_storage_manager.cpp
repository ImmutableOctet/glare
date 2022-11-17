#include "state_storage_manager.hpp"

//#include <engine/meta/component_storage.hpp>

namespace engine
{
	StateStorageManager::StateStorage& StateStorageManager::get_storage(EntityStateIndex index)
	{
		if (index >= states.size())
		{
			states.resize(static_cast<std::size_t>(index) + 1);
		}

		return states[index];
	}
}