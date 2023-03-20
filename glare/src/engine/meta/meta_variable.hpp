#pragma once

#include "types.hpp"
#include "meta_parsing_instructions.hpp"

// TODO: Forward declare JSON type.
#include <util/json.hpp>

#include <string_view>

namespace engine
{
	class MetaParsingContext;

	// Wrapper type around a name + meta_any value; used to represent a definition of a variable.
	struct MetaVariable
	{
		using Instructions = MetaParsingInstructions;

		MetaVariable(MetaSymbolID name, MetaAny&& value);
		MetaVariable(std::string_view name, MetaAny&& value);
		
		// NOTE: This overload does not allow for non-primitive type resolution.
		// (see overload taking `type_id`)
		MetaVariable(MetaSymbolID name, const util::json& value, const Instructions& instructions={});

		MetaVariable(MetaSymbolID name, const util::json& value, MetaType type, const Instructions& instructions={});
		MetaVariable(MetaSymbolID name, const util::json& value, MetaTypeID type_id, const Instructions& instructions={});

		// Utility overload that automates hash generation of `name`.
		// (see notes on primitive type limitations)
		MetaVariable(std::string_view name, const util::json& value, const Instructions& instructions={});

		// Utility overload that automates hash generation of `name`.
		MetaVariable(std::string_view name, const util::json& value, MetaTypeID type_id, const Instructions& instructions={});

		// Utility overload that automates hash generation of `name`.
		MetaVariable(std::string_view name, const util::json& value, MetaType type, const Instructions& instructions={});

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