#pragma once

#include "types.hpp"

#include <util/shared_storage_ref.hpp>

namespace engine
{
	// Type-safe reference to a shared object stored in an `EntityDescriptor`.
	template <typename ResourceType, typename IndexType=SharedStorageIndex>
	class EntityDescriptorShared : public util::SharedStorageRef<ResourceType, IndexType>
	{
		public:
			using SharedStorageRef = util::SharedStorageRef<ResourceType, IndexType>;

			using resource_type = SharedStorageRef::resource_type;
			using index_type    = SharedStorageRef::index_type;

			template <typename SharedStorageType>
			inline const ResourceType& get_from_storage(const SharedStorageType& shared_storage) const
			{
				return SharedStorageRef::get(shared_storage);
			}

			template <typename SharedStorageType>
			inline ResourceType& get_from_storage(SharedStorageType& shared_storage)
			{
				return SharedStorageRef::get(shared_storage);
			}

			template <typename EntityDescriptorType>
			inline const ResourceType& get(const EntityDescriptorType& descriptor) const
			{
				return get_from_storage(descriptor.get_shared_storage());
			}

			template <typename EntityDescriptorType>
			inline ResourceType& get(EntityDescriptorType& descriptor)
			{
				return get_from_storage(descriptor.get_shared_storage());
			}

			EntityDescriptorShared(IndexType index)
				: SharedStorageRef(index) {}

			template <typename EntityDescriptorType, typename Instance>
			EntityDescriptorShared(const EntityDescriptorType& descriptor, const Instance& instance)
				: SharedStorageRef(descriptor.get_shared_storage(), instance) {}

			EntityDescriptorShared(const EntityDescriptorShared&) = default;
			EntityDescriptorShared(EntityDescriptorShared&&) noexcept = default;

			EntityDescriptorShared& operator=(const EntityDescriptorShared&) = default;
			EntityDescriptorShared& operator=(EntityDescriptorShared&&) noexcept = default;

			bool operator==(const EntityDescriptorShared& value_in) const noexcept = default;
			bool operator!=(const EntityDescriptorShared& value_in) const noexcept = default;

			/*
			operator SharedStorageRef() const
			{
				return SharedStorageRef { this->index };
			}

			operator const SharedStorageRef&() const
			{
				return *this;
			}
			*/
	};
}