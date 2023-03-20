#include "indirect_meta_any.hpp"

#include "meta.hpp"
#include "shared_storage_interface.hpp"
#include "meta_evaluation_context.hpp"

#include <engine/entity/entity_descriptor.hpp>
#include <engine/entity/components/instance_component.hpp>
//#include <engine/resource_manager/entity_factory_data.hpp>

namespace engine
{
	IndirectMetaAny::IndirectMetaAny(MetaTypeID type_id, IndexType index, ChecksumType checksum)
		: type_id(type_id), index(index), checksum(checksum) {}

	IndirectMetaAny::IndirectMetaAny(const MetaType& type, IndexType index, ChecksumType checksum)
		: IndirectMetaAny(type.id(), index, checksum) {}

	IndirectMetaAny::IndirectMetaAny(const SharedStorageInterface& storage, const MetaAny& instance)
		: IndirectMetaAny(instance.type(), storage.get_index_safe(instance), storage.get_checksum()) {}

	IndirectMetaAny::IndirectMetaAny(SharedStorageInterface& storage, MetaAny&& resource)
		: type_id(resource.type().id()), index(storage.allocate_safe(std::move(resource))), checksum(storage.get_checksum()) {}

	bool IndirectMetaAny::validate_checksum(const SharedStorageInterface& storage) const
	{
		if (!checksum)
		{
			return true;
		}

		if (auto storage_checksum = storage.get_checksum())
		{
			return (storage_checksum == checksum);
		}

		return false;
	}

	MetaType IndirectMetaAny::get_type() const
	{
		return resolve(type_id);
	}

	MetaAny IndirectMetaAny::get(const SharedStorageInterface& storage) const
	{
		auto checksum_result = validate_checksum(storage);

		assert(checksum_result);

		if (!checksum_result)
		{
			return {};
		}

		return storage.get(type_id, index);
	}

	MetaAny IndirectMetaAny::get(SharedStorageInterface& storage)
	{
		auto checksum_result = validate_checksum(storage);

		assert(checksum_result);

		if (!checksum_result)
		{
			return {};
		}

		return storage.get(type_id, index);
	}

	MetaAny IndirectMetaAny::get(Registry& registry, Entity entity) const
	{
		// Const-cast used to avoid applying 'transitive const' to the underlying object.
		return const_cast<IndirectMetaAny*>(this)->get_impl(registry, entity);
	}

	MetaAny IndirectMetaAny::get_impl(Registry& registry, Entity entity)
	{
		if (entity == null)
		{
			return {};
		}

		auto* instance_component = registry.try_get<InstanceComponent>(entity);

		if (!instance_component)
		{
			return {};
		}

		auto& storage = instance_component->get_storage();

		return get(storage);
	}

	MetaAny IndirectMetaAny::get_indirect_value(Registry& registry, Entity entity) const
	{
		return get_indirect_value_impl(registry, entity);
	}

	MetaAny IndirectMetaAny::get_indirect_value(Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return get_indirect_value_impl(registry, entity, context);
	}

	MetaAny IndirectMetaAny::get_indirect_value(const MetaAny& instance, Registry& registry, Entity entity) const
	{
		return get_indirect_value_from_instance_impl(instance, registry, entity);
	}

	MetaAny IndirectMetaAny::get_indirect_value(const MetaAny& instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return get_indirect_value_from_instance_impl(instance, registry, entity, context);
	}

	MetaAny IndirectMetaAny::set_indirect_value(MetaAny& value, Registry& registry, Entity entity)
	{
		return set_indirect_value_impl(value, registry, entity);
	}

	MetaAny IndirectMetaAny::set_indirect_value(MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		return set_indirect_value_impl(value, registry, entity, context);
	}

	MetaAny IndirectMetaAny::set_indirect_value(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity)
	{
		return set_indirect_value_using_remote_impl(source, destination, registry, entity);
	}

	MetaAny IndirectMetaAny::set_indirect_value(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		return set_indirect_value_using_remote_impl(source, destination, registry, entity, context);
	}

	template <typename ...Args>
	MetaAny IndirectMetaAny::get_indirect_value_impl(Args&&... args) const
	{
		auto result = get(args...);

		if (!result)
		{
			return {};
		}

		if (auto underlying = try_get_underlying_value(result, args...))
		{
			return underlying;
		}

		return result;
	}

	template <typename ...Args>
	MetaAny IndirectMetaAny::get_indirect_value_from_instance_impl(const MetaAny& instance, Args&&... args) const
	{
		if (!instance)
		{
			return {};
		}

		auto result = get(args...);

		if (!result)
		{
			return {};
		}

		if (auto underlying = try_get_underlying_value(result, instance, args...))
		{
			return underlying;
		}

		return {}; // get_indirect_value_impl(args...);
	}

	template <typename ...Args>
	MetaAny IndirectMetaAny::set_indirect_value_impl(MetaAny& value, Args&&... args)
	{
		if (!value)
		{
			return {};
		}

		auto remote = get(args...);

		if (!remote)
		{
			return {};
		}

		if (auto result = try_get_underlying_value(remote, value, args...))
		{
			return result;
		}

		return remote;
	}

	template <typename ...Args>
	MetaAny IndirectMetaAny::set_indirect_value_using_remote_impl(MetaAny& source, MetaAny& destination, Args&&... args)
	{
		if (!source)
		{
			return {};
		}

		/*
		if (!destination)
		{
			return {};
		}
		*/

		auto remote = get(args...);

		if (!remote)
		{
			return {};
		}

		if (auto result = try_get_underlying_value(remote, source, destination, args...))
		{
			return result;
		}

		return remote;
	}
}