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
	struct MetaParsingInstructions;

	struct MetaTypeResolutionContext
	{
		// Maps aliases (`std::string`) to statically defined `std::string_view` objects queryable from `entt`.
		using AliasContainer = util::hash_map<std::string_view>; // std::string

		static MetaTypeResolutionContext generate(bool standard_mapping=true, bool reverse_mapping=true);
		
		static std::size_t generate_aliases
		(
			AliasContainer& container_out,
			std::string_view prefix,
			std::string_view suffix,
			bool standard_mapping=true, bool reverse_mapping=true,
			std::string_view opt_snake_prefix={}
		);

		// Attempts to resolve the `alias` specified using `container`.
		// If the input is not an alias, this will return an empty `std::string_view` instance.
		static std::string_view resolve_alias(const AliasContainer& container, std::string_view alias);

		// Attempts to resolve the type referenced by `name`.
		// The `name` argument can be either an alias (resolvable by `aliases`) or a regular type name.
		static MetaType get_type(const AliasContainer& aliases, std::string_view name, bool is_known_alias=false);

		// Attempts to resolve the underlying type for the `alias` specified.
		// If `alias` is not a registered alias in `alias_container`, this will return an empty/invalid `MetaType` instance.
		static MetaType get_type_from_alias(const AliasContainer& alias_container, std::string_view alias);

		// Attempts to resolve the input as a command alias.
		// If the input is not a valid command alias, this will return an empty `std::string_view` instance.
		inline std::string_view resolve_command_alias(std::string_view command_alias) const
		{
			return resolve_alias(command_aliases, command_alias);
		}

		// Attempts to resolve the type referenced by `command_name`.
		// The `command_name` argument can be either an alias or a regular command name.
		inline MetaType get_command_type(std::string_view command_name, bool is_known_alias=false) const
		{
			return get_type(command_aliases, command_name, is_known_alias);
		}

		// Attempts to resolve the underlying type for the command alias specified.
		// If `command_alias` is not a registered alias, this will return an empty/invalid `MetaType` instance.
		inline MetaType get_command_type_from_alias(std::string_view command_alias) const
		{
			return get_type_from_alias(command_aliases, command_alias);
		}

		// Attempts to resolve the input as a component alias.
		// If the input is not a valid component alias, this will return an empty `std::string_view` instance.
		inline std::string_view resolve_component_alias(std::string_view component_alias) const
		{
			return resolve_alias(component_aliases, component_alias);
		}

		// Attempts to resolve the type referenced by `component_name`.
		// The `component_name` argument can be either an alias or a regular component name.
		inline MetaType get_component_type(std::string_view component_name, bool is_known_alias=false) const
		{
			return get_type(component_aliases, component_name, is_known_alias);
		}

		// Attempts to resolve the underlying type for the component alias specified.
		// If `component_alias` is not a registered alias, this will return an empty/invalid `MetaType` instance.
		inline MetaType get_component_type_from_alias(std::string_view component_alias) const
		{
			return get_type_from_alias(component_aliases, component_alias);
		}

		// Attempts to resolve the input as an instruction alias.
		// If the input is not a valid instruction alias, this will return an empty `std::string_view` instance.
		inline std::string_view resolve_instruction_alias(std::string_view instruction_alias) const
		{
			return resolve_alias(instruction_aliases, instruction_alias);
		}

		// Attempts to resolve the type referenced by `instruction_name`.
		// The `instruction_name` argument can be either an alias or a regular instruction name.
		inline MetaType get_instruction_type(std::string_view instruction_name, bool is_known_alias=false) const
		{
			return get_type(instruction_aliases, instruction_name, is_known_alias);
		}

		// Attempts to resolve the underlying type for the instruction alias specified.
		// If `instruction_alias` is not a registered alias, this will return an empty/invalid `MetaType` instance.
		inline MetaType get_instruction_type_from_alias(std::string_view instruction_alias) const
		{
			return get_type_from_alias(instruction_aliases, instruction_alias);
		}

		MetaType get_type(std::string_view name, bool resolve_components=true, bool resolve_commands=true, bool resolve_instructions=false) const;
		MetaTypeID get_type_id(std::string_view name, bool resolve_components=true, bool resolve_commands=true, bool resolve_instructions=false) const;

		MetaType get_type(std::string_view name, const MetaParsingInstructions& instructions) const;
		MetaTypeID get_type_id(std::string_view name, const MetaParsingInstructions& instructions) const;

		// Map of component aliases to their underlying type name.
		AliasContainer component_aliases;

		// Map of command aliases to their underlying type name.
		AliasContainer command_aliases;

		// Map of entity instruction aliases to their underlying type name.
		AliasContainer instruction_aliases;
	};
}