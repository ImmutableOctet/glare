#include "common_members.hpp"

#include "hash.hpp"
#include "data_member.hpp"

namespace engine
{
	entt::meta_data get_entity_member(const MetaType& type)
	{
		using namespace engine::literals;

		return resolve_data_member_by_id(type, true, "entity"_hs);
	}

	entt::meta_data get_entity_member(const MetaAny& value)
	{
		if (!value)
		{
			return {};
		}

		const auto type = value.type();

		if (!type)
		{
			return {};
		}

		return get_entity_member(type);
	}

	bool has_entity_member(const MetaType& type)
	{
		return static_cast<bool>(get_entity_member(type));
	}

	bool has_entity_member(const MetaAny& value)
	{
		return static_cast<bool>(get_entity_member(value));
	}

	Entity try_get_entity(const MetaAny& value)
	{
		if (const auto data_member = get_entity_member(value))
		{
			if (const auto as_opaque_entity = data_member.get(value))
			{
				if (const auto as_entity = as_opaque_entity.try_cast<Entity>())
				{
					return *as_entity;
				}

				if (const auto as_entity_casted = as_opaque_entity.allow_cast<Entity>())
				{
					if (const auto as_entity = as_entity_casted.try_cast<Entity>())
					{
						return *as_entity;
					}
				}
			}
		}

		return null;
	}
}