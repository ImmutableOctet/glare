#pragma once

#include <engine/meta/types.hpp>
#include <engine/meta/indirect_meta_any.hpp>

namespace engine
{
	class SharedStorageInterface
	{
		public:
			inline virtual ~SharedStorageInterface() {}

			inline virtual std::optional<SharedStorageIndex> allocate(MetaTypeID type_id)
			{
				return std::nullopt;
			}

			inline virtual SharedStorageIndex allocate_safe(MetaTypeID type_id)
			{
				auto result = allocate(type_id);

				assert(result);

				return *result;
			}

			inline virtual std::optional<SharedStorageIndex> allocate(const MetaType& type)
			{
				if (!type)
				{
					return std::nullopt;
				}

				return allocate(type.id());
			}

			inline virtual SharedStorageIndex allocate_safe(const MetaType& type)
			{
				auto result = allocate(type);

				assert(result);

				return *result;
			}

			inline virtual std::optional<SharedStorageIndex> allocate(MetaAny&& resource)
			{
				return std::nullopt;
			}

			// Attempts to allocate an object of type `T`, then move `instance_to_move` into its place.
			// If allocation succeeds, this will return a `MetaAny` object storing an `IndirectMetaAny` handle.
			// If allocation fails, this routine will attempt to move `instance_to_move` into a new `MetaAny` instance.
			template <typename T>
			MetaAny allocate_or_meta(T&& instance_to_move)
			{
				// NOTE: We forward `instance_to_move` as an lvalue reference wrapped in a
				// `MetaAny` object to ensure proper move semantics within `allocate`.
				if (auto instance_wrapper = entt::forward_as_meta(instance_to_move))
				{
					if (auto instance_meta_type = instance_wrapper.type())
					{
						if (auto remote_index = allocate(std::move(instance_wrapper)))
						{
							return MetaAny
							{
								IndirectMetaAny
								{
									instance_meta_type.id(),
									*remote_index,
									get_checksum()
								}
							};
						}
					}
				}

				return MetaAny { std::move(instance_to_move) };
			}

			inline virtual SharedStorageIndex allocate_safe(MetaAny&& resource)
			{
				assert(resource);

				auto result = allocate(std::move(resource));

				assert(result);

				return *result;
			}

			inline virtual bool deallocate(MetaTypeID type_id, SharedStorageIndex index)
			{
				return false;
			}

			inline virtual bool deallocate(const MetaAny& instance) // MetaAny
			{
				return false;
			}

			inline virtual bool deallocate_safe(MetaTypeID type_id, SharedStorageIndex index)
			{
				assert(type_id);

				auto result = deallocate(type_id, index);

				assert(result);

				return result;
			}

			inline virtual bool deallocate_safe(const MetaAny& instance) // MetaAny
			{
				assert(instance);

				auto result = deallocate(instance);

				assert(result);

				return result;
			}

			inline virtual MetaAny get(MetaTypeID type_id, SharedStorageIndex index)
			{
				return {};
			}

			inline virtual MetaAny get(const MetaType& type, SharedStorageIndex index)
			{
				return get(type.id(), index);
			}

			inline virtual MetaAny get(MetaTypeID type_id, SharedStorageIndex index) const
			{
				return {};
			}

			inline virtual MetaAny get(const MetaType& type, SharedStorageIndex index) const
			{
				return get(type.id(), index);
			}

			inline virtual std::optional<SharedStorageIndex> get_index(const MetaAny& instance) const
			{
				return std::nullopt;
			}

			inline virtual SharedStorageIndex get_index_safe(const MetaAny& instance) const
			{
				assert(instance);
				assert(instance.type());

				auto result = get_index(instance);

				assert(result);

				return *result;
			}

			inline virtual MetaAny get_storage_as_any(MetaTypeID type_id)
			{
				return {};
			}

			inline virtual MetaAny get_storage_as_any(const MetaType& type)
			{
				return get_storage_as_any(type.id());
			}

			inline virtual SharedStorageInterface* get_storage(MetaTypeID type_id)
			{
				return this;
			}

			inline virtual SharedStorageInterface* get_storage(const MetaType& type)
			{
				return get_storage(type.id());
			}

			inline virtual MetaSymbolID get_checksum() const
			{
				return {};
			}

			inline virtual MetaSymbolID get_checksum(MetaTypeID type_id) const
			{
				return get_checksum();
			}

			inline virtual MetaSymbolID get_checksum(const MetaType& type) const
			{
				return get_checksum(type.id());
			}
	};

	// Attempts to call `SharedStorageInterface::allocate_or_meta` if `storage` exists.
	// If `storage` does not exist (points to `nullptr`), this will attempt to move `instance_to_move` into a new `MetaAny` object.
	template <typename T>
	MetaAny allocate_meta_any(T&& instance_to_move, SharedStorageInterface* storage=nullptr)
	{
		//static_assert((sizeof(T) > sizeof(double[2])));

		// TODO: Review whether this optimization should be used.
		if constexpr (true) // (sizeof(T) <= sizeof(double[2]))
		{
			if (storage)
			{
				return storage->allocate_or_meta(std::move(instance_to_move));
			}
		}

		return MetaAny { std::move(instance_to_move) };
	}
}