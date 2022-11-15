#pragma once

#include <engine/types.hpp>

#include <entt/entt.hpp>
//#include <entt/meta/meta.hpp>

#include <string_view>

namespace engine
{
	//using entt::meta_type;
	//using entt::meta_any;
	//using entt::meta_factory;

	using entt::type_info;
	//using entt::type_id;

	using entt::type_hash;

	using TypeInfo     = entt::type_info;
	using StringHash   = entt::id_type; // entt::hashed_string;
	using MetaSymbolID = entt::id_type;
	using MetaTypeID   = entt::id_type; // StringHash;
	using MetaType     = entt::meta_type;
	using MetaAny      = entt::meta_any;
}