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

	using TypeInfo       = entt::type_info;
	using MetaSymbolID   = entt::id_type;
	using MetaTypeID     = entt::id_type; // StringHash;
	using MetaFunctionID = entt::id_type; // StringHash;
	using MetaType       = entt::meta_type;
	using MetaAny        = entt::meta_any;
	using MetaHandle     = entt::meta_handle;
	using MetaFunction   = entt::meta_func;

	using MetaIDStorage = util::small_vector<MetaTypeID, 4>;
	using MetaRemovalDescription = MetaIDStorage; // <MetaType>
	using MetaStorageDescription = MetaIDStorage; // <MetaType>

	// TODO: Find a better location for this.
	using entt::resolve;

	// TODO: Move to a different source location.
	// Attempts to retrieve an instance of `Type` from `value`, passing it to `callback` if successful.
	template <typename Type, typename Callback>
	inline bool try_value(const MetaAny& value, Callback&& callback)
	{
		if (!value)
		{
			return false;
		}

        /*
		auto type = value.type();

		if (!type)
		{
			return false;
		}

		const auto type_id = type.id();
        */

		//if (type_id == entt::type_id<Type>().hash())
		//if ((type_id == entt::type_id<Type>().hash()) || (type_id == resolve<Type>().id()))
		{
			auto* value_raw = value.try_cast<Type>(); // const

			//assert(value_raw);

			if (!value_raw)
			{
				return false;
			}

			callback(*value_raw);

			return true;
		}

		return false;
	}
}