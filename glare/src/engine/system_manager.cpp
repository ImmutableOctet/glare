#include "system_manager.hpp"

namespace engine
{
	SystemManager::SystemManager(Service& service)
		: service(service)
	{}

	SystemManager::RawOpaquePointer SystemManager::add_system(System&& system)
	{
		RawOpaquePointer system_ptr = system.get();

		systems.push_back(std::move(system));

		return system_ptr;
	}
}