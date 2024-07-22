#pragma once

#include <engine/types.hpp>

#include <entt/meta/fwd.hpp>
//#include <entt/meta/meta.hpp>
//#include <entt/entt.hpp>

namespace engine
{
	using MetaSymbolID   = entt::id_type;
	using MetaTypeID     = entt::id_type; // StringHash;
	using MetaFunctionID = entt::id_type; // StringHash;
	using MetaType       = entt::meta_type;
	using MetaAny        = entt::meta_any;
	using MetaHandle     = entt::meta_handle;
	using MetaFunction   = entt::meta_func;
}