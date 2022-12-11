#pragma once

#include "meta_data_member.hpp"

#include <engine/entity_target.hpp>

namespace engine
{
	struct IndirectMetaDataMember
	{
		EntityTarget target;
		MetaDataMember data_member;

		// Forwards to the instance-only overload of `get` found in `data_member`.
		MetaAny get(const MetaAny& instance) const;

		// Attempts to resolve `target` from `entity`, then
		// executes the equivalent `get` method from `data_member`.
		// 
		// NOTE: Unlike `MetaDataMember`, this method can take `null` as a
		// (possibly) valid value for `entity`, due to the indirection from `target`.
		MetaAny get(Registry& registry, Entity entity=null, bool fallback_to_entity=false) const;
	};
}