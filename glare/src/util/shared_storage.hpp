#pragma once

#include "small_vector.hpp"

#include <tuple>
#include <optional>
#include <type_traits>
#include <iterator>
#include <cstdint>

#include <vector>

namespace util
{
	template <typename T, std::size_t preallocated = 4> // 8
	using DefaultSharedStorageContainer = std::vector<T>; // small_vector<T, preallocated>;

	using DefaultSharedStorageIndex = std::uint16_t; // std::size_t;

	template <typename ResourceType, template <typename> typename ContainerTypeTemplate=DefaultSharedStorageContainer, typename IndexType=DefaultSharedStorageIndex>
	class SharedStorageData
	{
		public:
			using container_type = ContainerTypeTemplate<ResourceType>;
			using resource_type  = ResourceType;
			using index_type     = IndexType;

			// Retrieves a temporary pointer to a meta-type description.
			inline const ResourceType& get(IndexType index) const
			{
				return (container[index]);
				//return container.at(index);
			}

			inline ResourceType& get(IndexType index)
			{
				return (container[index]);
				//return container.at(index);
			}
			
			inline IndexType allocate(ResourceType&& resource)
			{
				const auto next_index = get_next_index();

				container.emplace_back(std::move(resource));

				return next_index;
			}

			template <typename ...Args>
			inline ResourceType& allocate(Args&&... args)
			{
				return container.emplace_back(std::forward<Args>(args)...);
			}

			inline IndexType get_next_index() const
			{
				return static_cast<IndexType>(container.size());
			}

			inline std::optional<IndexType> get_index(const ResourceType& resource) const
			{
				const auto resource_it = &resource;

				const auto* begin = container.data(); // &(*container.cbegin());
				const auto* end   = (begin + container.size()); // &(*container.cend());

				if ((resource_it >= begin) && (resource_it < end))
				{
					return static_cast<IndexType>(std::distance(begin, resource_it));
				}

				return std::nullopt;
			}

			inline IndexType get_index_safe(const ResourceType& resource) const
			{
				const auto index = get_index(resource);

				assert(index.has_value());
				
				return *index;
			}

			inline IndexType get_index_unsafe(const ResourceType& resource) const
			{
				const auto* begin = container.data(); // cbegin();

				return static_cast<IndexType>(std::distance(begin, &resource));
			}

			inline const container_type& data() const
			{
				return container;
			}
		protected:
			container_type container;
	};

	template <typename StorageType, template <typename ResourceType> typename StorageData=StorageType::Data>
	class SharedStorageInterface : protected StorageType
	{
		protected:
			constexpr auto storage()
			{
				/*
				if constexpr (std::is_member_function_pointer_v<decltype(&StorageType::storage)>)
				{
					return StorageType::storage();
				}
				else
				*/
				{
					return static_cast<StorageType*>(this);
				}
			}

			constexpr const auto storage() const
			{
				/*
				if constexpr (std::is_member_function_pointer_v<decltype(&StorageType::storage)>)
				{
					return StorageType::storage();
				}
				else
				*/
				{
					return static_cast<const StorageType*>(this);
				}
			}
		public:
			using storage_type = StorageType;

			template <typename ResourceType>
			const auto& get_storage() const
			{
				using StorageMember = typename StorageData<ResourceType>::type;

				if constexpr (std::is_same_v<StorageMember, void>)
				{
					static_assert(std::integral_constant<ResourceType, false>::value, "Unsupported resource type.");
				}
				else
				{
					return StorageData<ResourceType>::get(storage());
				}
			}

			template <typename ResourceType>
			auto& get_storage()
			{
				using StorageMember = typename StorageData<ResourceType>::type;

				if constexpr (std::is_same_v<StorageMember, void>)
				{
					static_assert(std::integral_constant<ResourceType, false>::value, "Unsupported resource type.");
				}
				else
				{
					return StorageData<ResourceType>::get(storage());
				}
			}
					
			template <typename ResourceType>
			const auto& get(auto index) const
			{
				return get_storage<ResourceType>().get(index);
			}

			template <typename ResourceType>
			auto& get(auto index)
			{
				return get_storage<ResourceType>().get(index);
			}

			template <typename ResourceType, typename ...Args>
			inline ResourceType& allocate(Args&&... args)
			{
				return get_storage<ResourceType>().allocate(std::forward<Args>(args)...);
			}

			template <typename ResourceType>
			inline auto allocate(ResourceType&& resource)
			{
				return get_storage<ResourceType>().allocate(std::forward<ResourceType>(resource));
			}

			template <typename ResourceType>
			inline auto get_next_index() const
			{
				return get_storage<ResourceType>().get_next_index();
			}

			template <typename ResourceType>
			inline auto get_index(const ResourceType& resource) const
			{
				return get_storage<ResourceType>().get_index(resource);
			}

			template <typename ResourceType>
			inline auto get_index_unsafe(const ResourceType& resource) const
			{
				return get_storage<ResourceType>().get_index_unsafe(resource);
			}

			template <typename ResourceType>
			inline auto get_index_safe(const ResourceType& resource) const
			{
				return get_storage<ResourceType>().get_index_safe(resource);
			}

			template <typename ResourceType>
			inline const auto& data() const
			{
				return get_storage<ResourceType>().data();
			}
	};

	//template <typename SelfType>
	//class StoragePlaceholder : public SelfType {};
	//class StoragePlaceholder {};

	template <typename T>
	struct SharedStorageDescriptorImpl;

	template<typename CustomStorage, typename MemberType>
	struct SharedStorageDescriptorImpl<MemberType CustomStorage::*>
	{
		using type = typename MemberType::resource_type;
		//using ResourceType = type;
	};

	template <auto pointer_to_member=nullptr>
	struct SharedStorageDescriptor;

	template <auto pointer_to_member>
	struct SharedStorageDescriptor : SharedStorageDescriptorImpl<decltype(pointer_to_member)>
	{
		static constexpr auto member_ptr(auto self) { return pointer_to_member; }

		static constexpr auto& get(auto self)
		{
			auto member = member_ptr(self);

			return (self->*member);
		}
	};

	template <>
	struct SharedStorageDescriptor<nullptr>
	{
		using type = void;

		//static constexpr void* member_ptr() { return nullptr }; // std::nullptr_t
	};
	
	/*
		// Example of custom storage type for use with `SharedStorageInterface` or `RemoteSharedStorageType`:
		struct CustomStorageType
		{
			util::SharedStorageData<TypeA> a;
			util::SharedStorageData<TypeB> b;

			//template <typename ResourceType>
			//struct Data : util::SharedStorageDescriptor<CustomStorageType, ResourceType> {};

			template <typename ResourceType>
			struct Data : util::SharedStorageDescriptor<> {};
				
			template <> struct Data<TypeA> : util::SharedStorageDescriptor<&CustomStorageType::a> {};
			template <> struct Data<TypeB> : util::SharedStorageDescriptor<&CustomStorageType::b> {};
		};
	*/

	template <template <typename> typename ContainerTypeTemplate, typename IndexType=DefaultSharedStorageIndex, typename ...Resources>
	struct AutomaticSharedStorageImpl
	{
		protected:
			constexpr auto storage() { return this; }
			constexpr const auto storage() const { return this; }

			std::tuple<SharedStorageData<Resources, ContainerTypeTemplate, IndexType>...> resources = {};
		public:
			template <typename ResourceType>
			struct Data
			{
				using type = ResourceType;

				using StorageData = SharedStorageData<ResourceType, ContainerTypeTemplate, IndexType>;

				static constexpr auto& get(auto self)
				{
					return std::get<StorageData>(self->resources);
				}
			};
	};

	template <typename ...Resources>
	using AutomaticSharedStorage = AutomaticSharedStorageImpl<DefaultSharedStorageContainer, DefaultSharedStorageIndex, Resources...>;

	template <typename ...Resources>
	using SharedStorage = SharedStorageInterface<AutomaticSharedStorage<Resources...>>;
}