#pragma once

#include "types.hpp"

// TODO: Forward declare JSON type.
#include <util/json.hpp>

#include <string_view>

namespace engine
{
	// Wrapper type around a name + meta_any value; used to represent a definition of a variable.
	struct MetaVariable
	{
		MetaVariable(MetaSymbolID name, entt::meta_any&& value);
		
		// NOTE: This overload does not allow for non-primitive type resolution.
		// (see overload taking `type_id`)
		MetaVariable(MetaSymbolID name, const util::json& value);

		MetaVariable(MetaSymbolID name, const util::json& value, MetaType type);
		MetaVariable(MetaSymbolID name, const util::json& value, MetaTypeID type_id);

		// Utility overload that automates hash generation of `name`.
		// (see notes on primitive type limitations)
		MetaVariable(std::string_view name, const util::json& value);

		// Utility overload that automates hash generation of `name`.
		MetaVariable(std::string_view name, const util::json& value, MetaTypeID type_id);

		// Utility overload that automates hash generation of `name`.
		MetaVariable(std::string_view name, const util::json& value, MetaType type);

		MetaVariable(const MetaVariable&) = default;
		MetaVariable(MetaVariable&&) noexcept = default;

		MetaVariable& operator=(const MetaVariable&) = default;
		MetaVariable& operator=(MetaVariable&&) noexcept = default;

		MetaVariable() = default;

		MetaSymbolID name = {};
		//std::optional<MetaSymbolID> name = std::nullopt;

		MetaAny value;

		inline explicit operator bool() const
		{
			return static_cast<bool>(value);
		}

		inline bool has_name() const
		{
			return static_cast<bool>(name);
		}
	};
}