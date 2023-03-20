#include "instance_component.hpp"

#include <engine/resource_manager/entity_factory_data.hpp>
#include <engine/entity/entity_factory.hpp>
#include <engine/meta/shared_storage_interface.hpp>

namespace engine
{
	const EntityFactory& InstanceComponent::get_factory() const
	{
		assert(instance);

		return instance->factory;
	}

	const std::filesystem::path& InstanceComponent::instance_path() const
	{
		return get_factory().get_instance_path();
	}

	const EntityDescriptor& InstanceComponent::get_descriptor() const
	{
		return get_factory().get_descriptor();
	}

	SharedStorageInterface& InstanceComponent::get_storage()
	{
		auto& descriptor = const_cast<EntityDescriptor&>(get_descriptor());

		return descriptor.get_shared_storage();
	}

	const SharedStorageInterface& InstanceComponent::get_storage() const
	{
		const auto& descriptor = get_descriptor();

		return descriptor.get_shared_storage();
	}
}