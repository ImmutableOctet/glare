#pragma once

#include "types.hpp"

#include <util/hash_map.hpp>
#include <util/small_vector.hpp>

//#include <unordered_map>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>

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

			bool standard_mapping=true,
			bool reverse_mapping=true,
			
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

		// Attempts to resolve the input as a system alias.
		// If the input is not a valid system alias, this will return an empty `std::string_view` instance.
		inline std::string_view resolve_system_alias(std::string_view system_alias) const
		{
			return resolve_alias(system_aliases, system_alias);
		}

		// Attempts to resolve the type referenced by `system_name`.
		// The `system_name` argument can be either an alias or a regular system name.
		inline MetaType get_system_type(std::string_view system_name, bool is_known_alias=false) const
		{
			return get_type(system_aliases, system_name, is_known_alias);
		}

		// Attempts to resolve the underlying type for the system alias specified.
		// If `system_alias` is not a registered alias, this will return an empty/invalid `MetaType` instance.
		inline MetaType get_system_type_from_alias(std::string_view system_alias) const
		{
			return get_type_from_alias(system_aliases, system_alias);
		}

		// Attempts to resolve the input as a service alias.
		// If the input is not a valid service alias, this will return an empty `std::string_view` instance.
		inline std::string_view resolve_service_alias(std::string_view service_alias) const
		{
			return resolve_alias(service_aliases, service_alias);
		}

		// Attempts to resolve the type referenced by `service_name`.
		// The `service_name` argument can be either an alias or a regular service name.
		inline MetaType get_service_type(std::string_view service_name, bool is_known_alias = false) const
		{
			return get_type(service_aliases, service_name, is_known_alias);
		}

		// Attempts to resolve the underlying type for the service alias specified.
		// If `service_alias` is not a registered alias, this will return an empty/invalid `MetaType` instance.
		inline MetaType get_service_type_from_alias(std::string_view service_alias) const
		{
			return get_type_from_alias(service_aliases, service_alias);
		}

		MetaType get_type
		(
			std::string_view name,
			
			bool resolve_components=true,
			bool resolve_commands=true,
			bool resolve_instructions=false,
			bool resolve_systems=true,
			bool resolve_services=true
		) const;

		MetaTypeID get_type_id
		(
			std::string_view name,
			
			bool resolve_components=true,
			bool resolve_commands=true,
			bool resolve_instructions=false,
			bool resolve_systems=true,
			bool resolve_services=true
		) const;

		MetaType get_type(std::string_view name, const MetaParsingInstructions& instructions) const;
		MetaTypeID get_type_id(std::string_view name, const MetaParsingInstructions& instructions) const;

		// Returns true if `type`'s identifier is found in `global_namespace`.
		bool type_in_global_namespace(const MetaType& type) const;

		// Returns true if `type_id` is found in `global_namespace`.
		bool type_in_global_namespace(MetaTypeID type_id) const;

		/*
			Attempts to resolve the function referenced by `function_id`
			from one of the types referenced in `global_namespace`.
			
			If multiple types have functions matching the same ID,
			this will return the first result found.
			
			The function returned will always be the first known overload available from a type (if any).
		*/
		std::tuple<MetaType, MetaFunction> resolve_global_function(MetaFunctionID function_id) const;

		/*
			Attempts to resolve the data member referenced by `member_id`
			from one of the types referenced in `global_namespace`.
			
			If multiple types have data members matching the same ID,
			this will return the first result found.
		*/
		std::tuple<MetaType, entt::meta_data> resolve_global_data_member(MetaSymbolID member_id) const;

		/*
			Attempts to resolve the property referenced by `property_id` from
			one of the types referenced in `global_namespace`.
			
			If multiple types have properties matching the same ID,
			this will return the first result found.
		*/
		std::tuple<MetaType, entt::meta_prop> resolve_global_property(MetaSymbolID property_id) const;

		// A map of component aliases to their underlying type name.
		AliasContainer component_aliases;

		// A map of command aliases to their underlying type name.
		AliasContainer command_aliases;

		// A map of entity instruction aliases to their underlying type name.
		AliasContainer instruction_aliases;

		// A map of engine system aliases to their underlying type name.
		AliasContainer system_aliases;

		// A map of engine service aliases to their underlying type name.
		AliasContainer service_aliases;

		// Type identifiers included in global symbol resolution.
		// (Used for global function references, etc.)
		util::small_vector<MetaTypeID, 4> global_namespace;
	};
}