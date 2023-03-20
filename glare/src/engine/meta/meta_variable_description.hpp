#pragma once

#include "types.hpp"
#include "meta_variable_scope.hpp"

namespace engine
{
	struct MetaVariableDescription
	{
		MetaSymbolID      scope_local_name = {};
		MetaSymbolID      resolved_name    = {};
		MetaTypeID        type_id          = {};
		MetaVariableScope scope            = MetaVariableScope::Local;

		bool operator==(const MetaVariableDescription&) const noexcept = default;
		bool operator!=(const MetaVariableDescription&) const noexcept = default;
	};
}