#pragma once

//#include "types.hpp"

//#include <string>
#include <filesystem>

//#include "entity_descriptor.hpp"

namespace engine
{
	class EntityDescriptor;
	class EntityFactory;
	class SharedStorageInterface;

	struct EntityFactoryData;

	// Points to the factory data that was used to instantiate this entity.
	struct InstanceComponent
	{
		// NOTE: As long as this component is attached, it should be assumed that `instance` is valid.
		std::shared_ptr<const EntityFactoryData> instance;

		const EntityFactory& get_factory() const;
		const std::filesystem::path& instance_path() const;
		const EntityDescriptor& get_descriptor() const;

		SharedStorageInterface& get_storage();
		const SharedStorageInterface& get_storage() const;
	};
}