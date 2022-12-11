#include "indirect_meta_data_member.hpp"

namespace engine
{
	MetaAny IndirectMetaDataMember::get(const MetaAny& instance) const
	{
		return data_member.get(instance);
	}

	// Attempts to resolve `target` from `entity`, then
	// executes the equivalent `get` method from `data_member`.
	MetaAny IndirectMetaDataMember::get(Registry& registry, Entity entity, bool fallback_to_entity) const
	{
		auto target_entity = target.resolve(registry, entity);

		if (target_entity == null)
		{
			if (fallback_to_entity)
			{
				target_entity = entity;
			}
			else
			{
				return {};
			}
		}

		return data_member.get(registry, target_entity);
	}
}