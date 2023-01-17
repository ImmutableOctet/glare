#pragma once

#include "types.hpp"

#include <util/hash_map.hpp>

//#include <unordered_map>
#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace engine
{
	struct CommandParsingContext
	{
		// Maps aliases (`std::string`) to statically defined `std::string_view` objects queryable from `entt`.
		using AliasContainer = util::hash_map<std::string_view>; // std::string

		static CommandParsingContext generate(bool standard_mapping=true, bool reverse_mapping=true, std::string_view suffix="Command");
		static std::size_t generate(CommandParsingContext& context, bool standard_mapping=true, bool reverse_mapping=true, std::string_view suffix="Command");

		// Attempts to resolve the input as a command alias.
		// If the input is not an alias, this will return an empty `std::string_view` instance.
		std::string_view resolve_command_alias(std::string_view command_alias) const;

		// Attempts to resolve the type referenced by `command_name`.
		// The `command_name` argument can be either an alias or a regular command name.
		MetaType get_command_type(std::string_view command_name, bool is_known_alias=false) const;

		// Attempts to resolve the underlying type for the command alias specified.
		// If `command_alias` is not a registered alias, this will return an empty/invalid `MetaType` instance.
		// 
		// See also: `get_command_type`
		MetaType get_command_type_from_alias(std::string_view command_alias) const;

		// Map of command aliases to their underlying type name.
		AliasContainer command_aliases;
	};
}