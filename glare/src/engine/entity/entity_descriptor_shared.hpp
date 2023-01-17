#pragma once

#include "types.hpp"

namespace engine
{
	// Type-safe reference to a shared object stored in an `EntityDescriptor`.
	template <typename ResourceType, typename IndexType=SharedStorageIndex>
	class EntityDescriptorShared
	{
		public:
			template <typename EntityDescriptorType>
			inline const ResourceType& get(const EntityDescriptorType& descriptor) const
			{
				return descriptor.get_shared_storage().get<ResourceType>(index);
			}

			template <typename EntityDescriptorType>
			inline ResourceType& get(EntityDescriptorType& descriptor)
			{
				return descriptor.get_shared_storage().get<ResourceType>(index);
			}

			EntityDescriptorShared(IndexType index)
				: index(index) {}

			template <typename EntityDescriptorType, typename Instance>
			EntityDescriptorShared(const EntityDescriptorType& descriptor, const Instance& instance)
				: index(descriptor.shared_storage.get_index_safe(instance)) {}

			inline IndexType get_index() const
			{
				return index;
			}
		protected:
			IndexType index;
	};
}