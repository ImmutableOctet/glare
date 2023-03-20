#pragma once

#include "types.hpp"

//#include <engine/meta/meta.hpp>
#include <engine/meta/types.hpp>
#include <engine/meta/shared_storage_interface.hpp>

#include <util/shared_storage.hpp>

#include <tuple>
#include <optional>
#include <type_traits>

namespace engine
{
	template <typename T, std::size_t preallocated=4> // 8
	using EntitySharedContainerType = util::DefaultSharedStorageContainer<T, preallocated>;

	// TODO: Add better support for deallocation.
	template
	<
		typename ResourceType,
		template <typename> typename ContainerTypeTemplate=EntitySharedContainerType,
		typename IndexType=SharedStorageIndex
	>
	struct EntitySharedStorageData :
		public util::SharedStorageData<ResourceType, ContainerTypeTemplate, IndexType>,
		public SharedStorageInterface
	{
		public:
			using SharedStorageData = util::SharedStorageData<ResourceType, ContainerTypeTemplate, IndexType>;

			using SharedStorageData::allocate;
			//using SharedStorageInterface::allocate;

			using SharedStorageData::deallocate;
			//using SharedStorageInterface::deallocate;

			using SharedStorageInterface::allocate_safe;
			using SharedStorageData::get;
			using SharedStorageData::get_index;
			using SharedStorageData::get_index_safe;
			using SharedStorageInterface::get_index_safe;

			virtual ~EntitySharedStorageData() {}

			auto self_type() const
			{
				// NOTE: Direct usage of `entt::resolve` to avoid additional include.
				return entt::resolve<ResourceType>();
			}

			/*
			IndexType allocate(ResourceType&& resource)
			{
				return SharedStorageData::allocate(std::move(resource));
			}

			template <typename ...Args>
			ResourceType& allocate(Args&&... args)
			{
				return SharedStorageData::allocate(std::forward<Args>(args)...);
			}
			*/

			MetaAny get_storage_as_any(MetaTypeID type_id) override
			{
				if (type_id == self_type().id())
				{
					return entt::forward_as_meta<decltype(*this)>(*this);
				}

				return {};
			}

			MetaAny get(MetaTypeID type_id, SharedStorageIndex index) override
			{
				if (type_id != self_type().id())
				{
					return {};
				}

				auto instance_ptr = SharedStorageData::try_get(index);

				if (!instance_ptr)
				{
					return {};
				}

				return entt::forward_as_meta<decltype(*instance_ptr)>(*instance_ptr);
			}

			MetaAny get(MetaTypeID type_id, SharedStorageIndex index) const override
			{
				if (type_id != self_type().id())
				{
					return {};
				}

				auto instance_ptr = SharedStorageData::try_get(index);

				if (!instance_ptr)
				{
					return {};
				}

				return entt::forward_as_meta<decltype(*instance_ptr)>(*instance_ptr);
			}

			MetaAny get(const MetaType& type, SharedStorageIndex index) override
			{
				return get(type.id(), index);
			}

			MetaAny get(const MetaType& type, SharedStorageIndex index) const override
			{
				return get(type.id(), index);
			}

			std::optional<SharedStorageIndex> get_index(const MetaAny& instance) const override
			{
				if (!instance)
				{
					return std::nullopt;
				}

				const auto instance_type = instance.type();

				if (!instance_type || (instance_type != self_type()))
				{
					return std::nullopt;
				}

				const auto* raw_instance = instance.try_cast<ResourceType>();

				if (!raw_instance)
				{
					return std::nullopt;
				}

				return SharedStorageData::get_index(*raw_instance);
			}

		protected:
			std::optional<SharedStorageIndex> allocate(MetaTypeID type_id) override
			{
				if constexpr (std::is_default_constructible_v<ResourceType>)
				{
					if (type_id != self_type().id())
					{
						return std::nullopt;
					}

					const auto next_index = SharedStorageData::get_next_index();

					SharedStorageData::allocate();

					return next_index;
				}
				else
				{
					return std::nullopt;
				}
			}

			std::optional<SharedStorageIndex> allocate(MetaAny&& resource) override
			{
				if (resource.type() != self_type())
				{
					return std::nullopt;
				}

				auto* raw_data = resource.try_cast<ResourceType>();

				if (!raw_data)
				{
					return std::nullopt;
				}

				if constexpr (std::is_move_constructible_v<ResourceType>)
				{
					return SharedStorageData::allocate(std::move(*raw_data));
				}
				else if (std::is_default_constructible_v<ResourceType> && std::is_move_assignable_v<ResourceType>)
				{
					const SharedStorageIndex allocated_index = SharedStorageData::allocate();

					auto& instance = SharedStorageData::get(allocated_index);

					*instance = std::move(*raw_data);

					return allocated_index;
				}

				return std::nullopt;
			}

			bool deallocate(MetaTypeID type_id, SharedStorageIndex index) override
			{
				if (type_id != self_type().id())
				{
					return false;
				}

				return SharedStorageData::deallocate(index);
			}

			bool deallocate(const MetaAny& instance) override // MetaAny
			{
				if (!instance)
				{
					return false;
				}

				auto index = get_index(instance);

				if (!index)
				{
					return false;
				}

				return SharedStorageData::deallocate(*index);
			}
	};

	template <typename ...Resources>
	class EntityAutomaticSharedStorageImpl :
		public util::AutomaticSharedStorageImpl
		<
			EntitySharedContainerType,
			SharedStorageIndex,
			EntitySharedStorageData,
			Resources...
		>,
		public SharedStorageInterface
	{
		public:
			template <typename ...>
			friend class EntitySharedStorageImpl;

			using AutomaticSharedStorageImpl = util::AutomaticSharedStorageImpl
			<
				EntitySharedContainerType,
				SharedStorageIndex,
				EntitySharedStorageData,
				Resources...
			>;

			virtual ~EntityAutomaticSharedStorageImpl() {}

			template <typename T, std::size_t preallocated=4>
			using ContainerType = EntitySharedContainerType<T, preallocated>;

			using IndexType = SharedStorageIndex;

			template <typename ResourceType>
			using StorageData = EntitySharedStorageData<ResourceType, EntitySharedContainerType, IndexType>;

		protected:
			// TODO: Revisit per-instance checksums for entries in `AutomaticSharedStorageImpl::resources` field.
			MetaSymbolID checksum = {};

			void set_checksum(MetaSymbolID checksum)
			{
				if (!this->checksum)
				{
					this->checksum = checksum;
				}
			}

		private:
			template <typename Callback, bool use_full_type_resolution=true>
			static void get_storage_impl(auto& self, MetaTypeID resource_type_id, Callback&& callback)
			{
				std::apply
				(
					[&callback, resource_type_id](auto&&... resource_sequence)
					{
						(
							[&callback, resource_type_id](auto&& shared_data)
							{
								using shared_data_t = std::decay_t<decltype(shared_data)>;
								using resource_t = typename shared_data_t::resource_type;

								if constexpr (use_full_type_resolution)
								{
									// NOTE: Direct usage of `entt::resolve` to avoid additional include.
									if (auto resource_meta_type = entt::resolve<resource_t>())
									{
										if (resource_meta_type.id() == resource_type_id)
										{
											callback(std::forward<decltype(shared_data)>(shared_data));
										}
									}
								}
								else
								{
									if (entt::type_hash<resource_t>::value() == resource_type_id)
									{
										callback(std::forward<decltype(shared_data)>(shared_data));
									}
								}
							} (std::forward<decltype(resource_sequence)>(resource_sequence)), ...
						);
					},

					self.resources
				);
			}

		public:
			MetaSymbolID get_checksum() const override
			{
				return checksum;
			}

			// TODO: Implement type-specific checksums.
			MetaSymbolID get_checksum(const MetaType& type) const override
			{
				return get_checksum();
			}

			// TODO: Implement type-specific checksums.
			MetaSymbolID get_checksum(MetaTypeID type_id) const override
			{
				return get_checksum();
			}

			SharedStorageInterface* get_storage(MetaTypeID type_id) override
			{
				SharedStorageInterface* output = nullptr;

				get_storage_impl
				(
					*this, type_id,

					[&output](auto& storage_data)
					{
						if (!output)
						{
							output = &storage_data;
						}
					}
				);

				return output;
			}

			SharedStorageInterface* get_storage(const MetaType& type) override
			{
				return get_storage(type.id());
			}

			MetaAny get_storage_as_any(MetaTypeID type_id) override
			{
				auto output = MetaAny {};

				get_storage_impl
				(
					*this, type_id,

					[&output](auto& storage_data)
					{
						if (!output)
						{
							output = entt::forward_as_meta<decltype(storage_data)>(storage_data);
						}
					}
				);

				return output;
			}

			MetaAny get_storage_as_any(const MetaType& type) override
			{
				return get_storage_as_any(type.id());
			}

			std::optional<SharedStorageIndex> allocate(MetaAny&& resource) override
			{
				if (!resource)
				{
					return std::nullopt;
				}

				std::optional<SharedStorageIndex> result = std::nullopt;

				const auto resource_type = resource.type();

				get_storage_impl
				(
					*this, resource_type.id(),

					[&resource, &result](auto& storage_data)
					{
						if (!result)
						{
							auto as_storage_interface = static_cast<SharedStorageInterface*>(&storage_data);

							if (auto allocation_attempt = as_storage_interface->allocate(std::move(resource)))
							{
								result = allocation_attempt;
							}
						}
					}
				);

				return result;
			}

			std::optional<SharedStorageIndex> allocate(MetaTypeID type_id) override
			{
				std::optional<SharedStorageIndex> result = std::nullopt;

				get_storage_impl
				(
					*this, type_id,

					[&type_id, &result](auto& storage_data)
					{
						if (!result)
						{
							auto as_storage_interface = static_cast<SharedStorageInterface*>(&storage_data);

							if (auto allocation_attempt = as_storage_interface->allocate(type_id))
							{
								result = allocation_attempt;
							}
						}
					}
				);

				return result;
			}

			bool deallocate(MetaTypeID type_id, SharedStorageIndex index) override
			{
				bool result = false;

				get_storage_impl
				(
					*this, type_id,

					[type_id, index, &result](auto& storage_data)
					{
						if (!result)
						{
							auto as_storage_interface = static_cast<SharedStorageInterface*>(&storage_data);

							if (auto deallocation_attempt = as_storage_interface->deallocate(type_id, index))
							{
								result = deallocation_attempt;
							}
						}
					}
				);

				return result;
			}

			bool deallocate(const MetaAny& instance) override // MetaAny
			{
				if (!instance)
				{
					return false;
				}

				const auto instance_type = instance.type();

				bool result = false;

				get_storage_impl
				(
					*this, instance_type.id(),

					[&instance, &result](auto& storage_data)
					{
						if (!result)
						{
							auto as_storage_interface = static_cast<SharedStorageInterface*>(&storage_data);

							if (auto deallocation_attempt = as_storage_interface->deallocate(instance))
							{
								result = deallocation_attempt;
							}
						}
					}
				);

				return result;
			}

			MetaAny get(MetaTypeID type_id, SharedStorageIndex index) override
			{
				auto result = MetaAny {};

				get_storage_impl
				(
					*this, type_id,

					[type_id, index, &result](auto& storage_data)
					{
						if (!result)
						{
							if (auto get_attempt = storage_data.get(type_id, index))
							{
								result = std::move(get_attempt);
							}
						}
					}
				);

				return result;
			}

			MetaAny get(MetaTypeID type_id, SharedStorageIndex index) const override
			{
				auto result = MetaAny {};

				get_storage_impl
				(
					*this, type_id,

					[type_id, index, &result](auto& storage_data)
					{
						if (!result)
						{
							if (auto get_attempt = storage_data.get(type_id, index))
							{
								result = std::move(get_attempt);
							}
						}
					}
				);

				return result;
			}

			MetaAny get(const MetaType& type, SharedStorageIndex index) override
			{
				return get(type.id(), index);
			}

			MetaAny get(const MetaType& type, SharedStorageIndex index) const override
			{
				return get(type.id(), index);
			}

			std::optional<SharedStorageIndex> get_index(const MetaAny& instance) const override
			{
				std::optional<SharedStorageIndex> result = std::nullopt;

				const auto instance_type = instance.type();

				if (!instance_type)
				{
					return std::nullopt;
				}

				const auto type_id = instance_type.id();

				get_storage_impl
				(
					*this, type_id,

					[&instance, &result](auto& storage_data)
					{
						if (!result)
						{
							result = storage_data.get_index(instance);
						}
					}
				);

				return result;
			}
	};

	template <typename ...Resources>
	using EntitySharedStorageBase = util::SharedStorageInterface<EntityAutomaticSharedStorageImpl<Resources...>>;

	template <typename ...Resources>
	class EntitySharedStorageImpl :
		public EntitySharedStorageBase<Resources...>,
		public SharedStorageInterface
	{
		public:
			using BaseType = EntitySharedStorageBase<Resources...>;

			EntitySharedStorageImpl() = default;

			EntitySharedStorageImpl(MetaSymbolID checksum)
			{
				if (checksum)
				{
					set_checksum(checksum);
				}
			}

			EntitySharedStorageImpl(const EntitySharedStorageImpl&) = delete;
			EntitySharedStorageImpl(EntitySharedStorageImpl&&) noexcept = default;

			//virtual ~EntitySharedStorageImpl() {}

			EntitySharedStorageImpl& operator=(const EntitySharedStorageImpl&) = delete;
			EntitySharedStorageImpl& operator=(EntitySharedStorageImpl&&) noexcept = default; // delete;

			using BaseType::get_storage;
			using BaseType::allocate;
			using BaseType::deallocate;
			using SharedStorageInterface::allocate_safe;
			using BaseType::get;
			using BaseType::get_index;
			using BaseType::get_index_safe;
			using SharedStorageInterface::get_index_safe;

			const auto& get_shared_storage() const
			{
				return *this;
			}

			auto& get_shared_storage()
			{
				return *this;
			}

			MetaAny get_storage_as_any(MetaTypeID type_id) override
			{
				return this->storage()->get_storage_as_any(type_id);
			}

			MetaAny get_storage_as_any(const MetaType& type) override
			{
				return this->storage()->get_storage_as_any(type);
			}

			SharedStorageInterface* get_storage(MetaTypeID type_id) override
			{
				return this->storage()->get_storage(type_id);
			}

			SharedStorageInterface* get_storage(const MetaType& type) override
			{
				return this->storage()->get_storage(type);
			}

			/*
			template <typename ResourceType, typename ...Args>
			inline ResourceType& allocate(Args&&... args)
			{
				return BaseType::template get_storage<ResourceType>().allocate(std::forward<Args>(args)...);
			}

			template <typename ResourceType>
			inline auto allocate(ResourceType&& resource)
			{
				return BaseType::template get_storage<ResourceType>().allocate(std::forward<ResourceType>(resource));
			}
			*/

			std::optional<SharedStorageIndex> allocate(const MetaType& type) override
			{
				return this->storage()->allocate(type);
			}

			MetaAny get(MetaTypeID type_id, SharedStorageIndex index) override
			{
				return this->storage()->get(type_id, index);
			}

			MetaAny get(const MetaType& type, SharedStorageIndex index) override
			{
				return this->storage()->get(type, index);
			}

			MetaAny get(MetaTypeID type_id, SharedStorageIndex index) const override
			{
				return this->storage()->get(type_id, index);
			}

			MetaAny get(const MetaType& type, SharedStorageIndex index) const override
			{
				return this->storage()->get(type, index);
			}

			std::optional<SharedStorageIndex> get_index(const MetaAny& instance) const override
			{
				return this->storage()->get_index(instance);
			}

			MetaSymbolID get_checksum() const override
			{
				return this->storage()->get_checksum();
			}

			MetaSymbolID get_checksum(MetaTypeID type_id) const
			{
				return this->storage()->get_checksum(type_id);
			}

			MetaSymbolID get_checksum(const MetaType& type) const
			{
				return this->storage()->get_checksum(type);
			}
		protected:
			std::optional<SharedStorageIndex> allocate(MetaAny&& resource) override
			{
				return this->storage()->allocate(std::move(resource));
			}

			std::optional<SharedStorageIndex> allocate(MetaTypeID type_id) override
			{
				return this->storage()->allocate(type_id);
			}

			bool deallocate(MetaTypeID type_id, SharedStorageIndex index) override
			{
				return this->storage()->deallocate(type_id, index);
			}

			bool deallocate(const MetaAny& instance) override // MetaAny
			{
				return this->storage()->deallocate(instance);
			}

			void set_checksum(MetaSymbolID checksum)
			{
				this->storage()->set_checksum(checksum);
			}
	};
}