#pragma once

#include "meta_data_member.hpp"

#include <engine/entity/entity_target.hpp>

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

		// Wrapper for `get`; added for reflection purposes.
		inline MetaAny get_indirect_value(Registry& registry, Entity entity) const
		{
			return get(registry, entity);
		}

		bool has_member() const;
		entt::meta_data get_data() const;

		bool has_type() const;
		MetaType get_type() const;

		bool has_member_type() const;
		MetaType get_member_type() const;

		bool operator==(const IndirectMetaDataMember&) const noexcept = default;
		bool operator!=(const IndirectMetaDataMember&) const noexcept = default;

		bool operator==(const MetaDataMember& value) const noexcept
		{
			return ((target.is_self_targeted()) && (data_member == value));
		}

		inline bool operator!=(const MetaDataMember& value) const noexcept
		{
			return !operator==(value);
		}
	};
}