#pragma once

#include "types.hpp"

namespace engine
{
	entt::meta_data get_entity_member(const MetaType& type);
	entt::meta_data get_entity_member(const MetaAny& value);
	
	bool has_entity_member(const MetaType& type);
	bool has_entity_member(const MetaAny& value);

	Entity try_get_entity(const MetaAny& value);
}