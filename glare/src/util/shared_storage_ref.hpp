#pragma once

#include <cstdint>

namespace util
{
	//using DefaultSharedStorageIndex = std::uint16_t; // std::size_t;

	// Type-safe reference to a shared object stored in a `SharedStorageInterface`.
	template <typename ResourceType, typename IndexType=std::uint16_t> // DefaultSharedStorageIndex
	class SharedStorageRef
	{
		public:
			using resource_type = ResourceType;
			using index_type    = IndexType;

			template <typename SharedStorageType>
			inline const ResourceType& get(const SharedStorageType& shared_storage) const
			{
				return shared_storage.get<ResourceType>(index);
			}

			template <typename SharedStorageType>
			inline ResourceType& get(SharedStorageType& shared_storage)
			{
				return shared_storage.get<ResourceType>(index);
			}

			SharedStorageRef(IndexType index)
				: index(index) {}

			template <typename SharedStorageType, typename Instance>
			SharedStorageRef(const SharedStorageType& shared_storage, const Instance& instance)
				: index(static_cast<IndexType>(shared_storage.get_index_safe(instance))) {}

			SharedStorageRef(const SharedStorageRef&) = default;
			SharedStorageRef(SharedStorageRef&&) noexcept = default;

			SharedStorageRef& operator=(const SharedStorageRef&) = default;
			SharedStorageRef& operator=(SharedStorageRef&&) noexcept = default;

			inline IndexType get_index() const
			{
				return index;
			}

			bool operator==(const SharedStorageRef& value_in) const noexcept = default;
			bool operator!=(const SharedStorageRef& value_in) const noexcept = default;
		protected:
			IndexType index;
	};
}