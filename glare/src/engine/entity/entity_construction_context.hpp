#pragma once

#include <engine/types.hpp>

namespace engine
{
	//class World;
	class ResourceManager;

	class Service;
	class SystemManagerInterface;

	struct EntityConstructionContext
	{
		Registry& registry;
		ResourceManager& resource_manager;

		//std::filesystem::path instance_path;

		Entity parent = null;

		// If this field is left as `null`, a factory will
		// generate an appropriate Entity instance.
		Entity opt_entity_out = null;

		Service* opt_service = nullptr;
		SystemManagerInterface* opt_system_manager = nullptr;
	};
}