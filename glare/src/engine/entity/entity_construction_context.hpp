#pragma once

#include <engine/types.hpp>

namespace engine
{
	//class World;
	class ResourceManager;

	//template <typename ServiceType>
	struct EntityConstructionContext
	{
		//using ServiceType = World;
		//ServiceType& service;

		Registry& registry;
		ResourceManager& resource_manager;

		//std::filesystem::path instance_path;

		Entity parent = null;

		// If this field is left as `null`, a factory will
		// generate an appropriate Entity instance.
		Entity opt_entity_out = null;
	};
}