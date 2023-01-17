#pragma once

#include <engine/types.hpp>

#include <util/small_vector.hpp>

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
	using MetaSymbolID = entt::id_type;
	using MetaTypeID   = entt::id_type; // StringHash;
	using MetaType     = entt::meta_type;
	using MetaAny      = entt::meta_any;

	using MetaIDStorage = util::small_vector<MetaTypeID, 4>;
	using MetaRemovalDescription = MetaIDStorage; // <MetaType>
	using MetaStorageDescription = MetaIDStorage; // <MetaType>

	using MetaStorage = util::small_vector<MetaAny, 3>; // 4
	using MetaSymbolStorage = util::small_vector<MetaSymbolID, 3>; // 4

	struct MetaAnyParseInstructions
	{
		bool resolve_symbol                      : 1 = true;
		bool strip_quotes                        : 1 = true;
		bool fallback_to_string                  : 1 = true;
		bool resolve_component_member_references : 1 = false;
		//bool allow_value_resolution_commands     : 1 = true;
	};
}