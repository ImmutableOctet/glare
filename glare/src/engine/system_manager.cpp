#include "system_manager.hpp"

namespace engine
{
	SystemManager::SystemManager(Service& service)
		: service(service)
	{}

	SystemManager::System& SystemManager::add_system(SystemID system_id, System&& system)
	{
		auto& any_instance = (systems[system_id] = std::move(system));

		return any_instance;
	}
}