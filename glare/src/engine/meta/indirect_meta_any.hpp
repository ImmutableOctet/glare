#pragma once

#include "types.hpp"

#include <util/shared_storage_ref.hpp>
#include <utility>

namespace engine
{
	class SharedStorageInterface;
	struct MetaEvaluationContext;

	class IndirectMetaAny
	{
		public:
			using IndexType = SharedStorageIndex;
			using ChecksumType = MetaSymbolID;

			IndirectMetaAny(MetaTypeID type_id, IndexType index, ChecksumType checksum={});
			IndirectMetaAny(const MetaType& type, IndexType index, ChecksumType checksum={});

			IndirectMetaAny(const SharedStorageInterface& storage, const MetaAny& instance);
			IndirectMetaAny(SharedStorageInterface& storage, MetaAny&& resource);

			template <typename StorageType, typename ResourceType>
			IndirectMetaAny(const StorageType& storage, const ResourceType& remote_resource, bool validate_type_id=true)
				: IndirectMetaAny(entt::type_hash<ResourceType>::value(), storage.get_index_safe(remote_resource), storage.get_checksum())
			{
				if (validate_type_id)
				{
					if (auto type = resolve<ResourceType>())
					{
						if (type_id != type.id())
						{
							type_id = type.id();
						}
					}
				}
			}

			template <typename ResourceType, typename IndexTypeIn>
			IndirectMetaAny(const util::SharedStorageRef<ResourceType, IndexTypeIn>& remote_resource, MetaTypeID type_id, ChecksumType checksum={})
				: type_id(type_id), index(static_cast<IndexType>(remote_resource.get_index())), checksum(checksum)
			{
				static_assert((sizeof(IndexTypeIn) <= sizeof(IndexType)), "Index narrowing not allowed.");
			}

			template <typename ResourceType, typename IndexTypeIn>
			IndirectMetaAny(const util::SharedStorageRef<ResourceType, IndexTypeIn>& remote_resource, const MetaType& type, ChecksumType checksum={})
				: IndirectMetaAny(remote_resource, type.id(), checksum)
			{}

			template <typename ResourceType, typename IndexTypeIn>
			IndirectMetaAny(const util::SharedStorageRef<ResourceType, IndexTypeIn>& remote_resource)
				: IndirectMetaAny(remote_resource, resolve<ResourceType>())
			{}

			IndirectMetaAny(const IndirectMetaAny&) = default;
			IndirectMetaAny(IndirectMetaAny&&) noexcept = default;

			IndirectMetaAny& operator=(const IndirectMetaAny&) = default;
			IndirectMetaAny& operator=(IndirectMetaAny&&) noexcept = default;

			// TODO: Revisit whether checksum should be included in comparison logic.
			bool operator==(const IndirectMetaAny&) const noexcept = default;
			bool operator!=(const IndirectMetaAny&) const noexcept = default;

			// Compares the internal checksum of `storage` to the checksum of this handle.
			// If the checksum of this handle has not been set (0), this member-function will always return true.
			bool validate_checksum(const SharedStorageInterface& storage) const;

			// Resolves the type of the remote `MetaAny` object.
			MetaType get_type() const;

			MetaAny get(const SharedStorageInterface& storage) const;
			MetaAny get(SharedStorageInterface& storage);

			// NOTE: This overload is for retrieving the remote `MetaAny` object from `entity`'s descriptor.
			// To resolve the underlying value of that `MetaAny` instance, use `get_indirect_value`, or the formal `try_get_underlying_value` API.
			MetaAny get(Registry& registry, Entity entity) const;

			inline IndexType get_index() const
			{
				return index;
			}

			// A type ID for the remote `MetaAny` object.
			inline MetaTypeID get_type_id() const
			{
				return type_id;
			}

			// An optional checksum associated with the container that owns the remote `MetaAny` object.
			// If a checksum has not been assigned to this handle, this value will be defaulted to 0.
			inline ChecksumType get_checksum() const
			{
				return checksum;
			}

			// Reflection API:
			
			// Retrieves the underlying value of the remote `MetaAny` object stored in `entity`'s descriptor.
			MetaAny get_indirect_value(Registry& registry, Entity entity) const;

			// Retrieves the underlying value of the remote `MetaAny` object stored in `entity`'s descriptor.
			MetaAny get_indirect_value(Registry& registry, Entity entity, const MetaEvaluationContext& context) const;

			// Attempts to retrieve the underlying value of `instance` using the remote `MetaAny` object stored in `entity`'s descriptor.
			MetaAny get_indirect_value(const MetaAny& instance, Registry& registry, Entity entity) const;

			// Attempts to retrieve the underlying value of `instance` using the remote `MetaAny` object stored in `entity`'s descriptor.
			MetaAny get_indirect_value(const MetaAny& instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const;

			// Attempts to formally set the remote `MetaAny` object stored in `entity`'s descriptor to `value`.
			// 
			// NOTE: Direct assignment of the remote `MetaAny` object is currently unsupported.
			// (i.e. via `MetaAny::operator=` as opposed to "operator="_hs dispatch)
			MetaAny set_indirect_value(MetaAny& value, Registry& registry, Entity entity);

			// Attempts to formally set the remote `MetaAny` object stored in `entity`'s descriptor to `value`.
			// 
			// NOTE: Direct assignment of the remote `MetaAny` object is currently unsupported.
			// (i.e. via `MetaAny::operator=` as opposed to "operator="_hs dispatch)
			MetaAny set_indirect_value(MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context);

			// Attempts to formally set `source` to `destination` using the remote `MetaAny`.
			MetaAny set_indirect_value(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity);

			// Attempts to formally set `source` to `destination` using the remote `MetaAny`.
			MetaAny set_indirect_value(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity, const MetaEvaluationContext& context);

		protected:
			MetaTypeID type_id;
			IndexType index;
			ChecksumType checksum;

		private:
			MetaAny get_impl(Registry& registry, Entity entity);

			// Added to simplify generic programming in implementation; not actually used for reflection dispatch.
			inline MetaAny get(Registry& registry, Entity entity, const MetaEvaluationContext& context) const
			{
				return get(registry, entity);
			}

			// Added to simplify generic programming in implementation; not actually used for reflection dispatch.
			inline MetaAny get_impl(Registry& registry, Entity entity, const MetaEvaluationContext& context)
			{
				return get_impl(registry, entity);
			}

			// Reflection implementation:
			template <typename ...Args>
			MetaAny get_indirect_value_impl(Args&&... args) const;

			template <typename ...Args>
			MetaAny get_indirect_value_from_instance_impl(const MetaAny& instance, Args&&... args) const;

			template <typename ...Args>
			MetaAny set_indirect_value_impl(MetaAny& value, Args&&... args);

			template <typename ...Args>
			MetaAny set_indirect_value_using_remote_impl(MetaAny& source, MetaAny& destination, Args&&... args);
	};
}