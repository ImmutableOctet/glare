#include "runtime_traits.hpp"

#include "hash.hpp"
#include "indirect_meta_any.hpp"

#include <cstdint>

//#include <string>
//#include <string_view>

namespace engine
{
	bool type_is_primitive(const MetaType& type, bool strings_as_primitives)
	{
		if (!type)
		{
			return false;
		}

		return type_is_primitive(type.id(), strings_as_primitives); // (type.is_arithmetic()) || ...
	}

	bool type_is_primitive(MetaTypeID type_id, bool strings_as_primitives)
	{
		using namespace engine::literals;

		switch (type_id)
		{
			case entt::type_hash<bool>::value():
			case entt::type_hash<std::int64_t>::value():
			case entt::type_hash<std::int32_t>::value():
			case entt::type_hash<std::int16_t>::value():
			case entt::type_hash<std::int8_t>::value():
			case entt::type_hash<std::uint64_t>::value():
			case entt::type_hash<std::uint32_t>::value():
			case entt::type_hash<std::uint16_t>::value():
			case entt::type_hash<std::uint8_t>::value():
			case entt::type_hash<long double>::value():
			case entt::type_hash<double>::value():
			case entt::type_hash<float>::value():
				return true;
		}

		if (strings_as_primitives)
		{
			switch (type_id)
			{
				case "string"_hs: // entt::type_hash<std::string>::value():
				case "string_view"_hs: // entt::type_hash<std::string_view>::value():
					return true;
			}
		}

		return false;
	}

    bool value_is_primitive(const MetaAny& value, bool strings_as_primitives)
	{
		if (!value)
		{
			return false;
		}

		const auto type = value.type();

		return type_is_primitive(type, strings_as_primitives);
	}

	bool value_has_indirection(const MetaAny& value, bool bypass_indirect_meta_any)
	{
		using namespace engine::literals;

		if (!value)
		{
			return {};
		}

		auto type = value.type();

		if (bypass_indirect_meta_any)
		{
			// TODO: Look into adding support for `MetaValueOperation` as well. (May need to be a shallow check)
			if (type.id() == "IndirectMetaAny"_hs)
			{
				if (const auto* as_indirect = value.try_cast<IndirectMetaAny>())
				{
					return type_has_indirection(as_indirect->get_type());
				}
			}
		}

		return type_has_indirection(type);
	}

	bool type_has_indirection(const MetaType& type)
	{
		using namespace engine::literals;

		if (!type)
		{
			return false;
		}

		if (type.func("operator()"_hs))
		{
			return true;
		}

		if (type.func("operator="_hs))
		{
			return true;
		}

		if (type.prop("optional"_hs))
		{
			return true;
		}

		return false;
	}

	bool type_has_indirection(MetaTypeID type_id)
	{
		auto type = resolve(type_id);

		if (!type)
		{
			return false;
		}

		return type_has_indirection(type);
	}

	bool type_is_system(const MetaType& type)
	{
		using namespace engine::literals;

		return ((type) && (type.prop("system"_hs)));
	}

	bool type_is_system(const MetaTypeID type_id)
	{
		if (auto type = resolve(type_id))
		{
			return type_is_system(type);
		}

		return false;
	}

	// Returns true if the `value` specified references a 'system'.
	bool value_is_system(const MetaAny& value)
	{
		if (!value)
		{
			return false;
		}

		if (auto type = value.type())
		{
			return type_is_system(type);
		}

		return false;
	}

	bool type_has_global_namespace_flag(const MetaType& type)
	{
		using namespace engine::literals;

		return ((type) && (type.prop("global namespace"_hs)));
	}

	bool type_has_global_namespace_flag(MetaTypeID type_id)
	{
		if (type_id)
		{
			if (auto type = resolve(type_id))
			{
				return type_has_global_namespace_flag(type);
			}
		}

		return false;
	}

	bool type_is_component(const MetaType& type)
	{
		using namespace engine::literals;

		return ((type) && (type.prop("component"_hs)));
	}

	bool type_is_component(MetaTypeID type_id)
	{
		if (type_id)
		{
			if (auto type = resolve(type_id))
			{
				return type_is_component(type);
			}
		}

		return false;
	}

	bool type_is_history_component(const MetaType& type)
	{
		using namespace engine::literals;

		return ((type) && (type.prop("history_component"_hs)));
	}

	bool type_is_history_component(MetaTypeID type_id)
	{
		if (type_id)
		{
			if (auto type = resolve(type_id))
			{
				return type_is_history_component(type);
			}
		}

		return false;
	}
}