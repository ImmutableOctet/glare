#pragma once

#include <engine/meta/meta_variable_scope.hpp>

#include <string>
#include <string_view>

namespace engine
{
	struct StaticVariableMutation
	{
		std::string variable_name;

		//MetaVariableScope variable_scope = MetaVariableScope::Global;

		inline std::string_view get_name() const
		{
			return std::string_view { variable_name };
		}

		inline MetaVariableScope get_scope() const
		{
			//return variable_scope;

			return MetaVariableScope::Global;
		}

		bool operator==(const StaticVariableMutation&) const = default;
		bool operator!=(const StaticVariableMutation&) const = default;
	};
}