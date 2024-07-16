#pragma once

#include "types.hpp"

#include "runtime_traits.hpp"

#include <util/hash_map.hpp>
#include <util/small_vector.hpp>
#include <util/string.hpp>

//#include <unordered_map>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>

namespace engine
{
	struct MetaParsingInstructions;

	namespace impl
	{
		template <typename Callback, typename ShortNameFn>
		void generate_simplified_type_names
		(
			Callback&& callback,
			ShortNameFn&& short_name_fn,

			std::string_view prefix,
			std::string_view suffix,
			std::string_view opt_snake_prefix={}
		)
		{
			for (const auto& type_entry : entt::resolve())
			{
				const auto& type_id = type_entry.first;
				const auto& type = type_entry.second;

				const auto& type_info = type.info();
				const auto type_name = type_info.name();

				if (!suffix.empty())
				{
					if (!type_name.ends_with(suffix))
					{
						continue;
					}
				}

				auto short_name = short_name_fn(type_name);

				if (!prefix.empty())
				{
					if (!short_name.starts_with(prefix))
					{
						continue;
					}
				}

				auto short_name_no_prefix = short_name.substr(prefix.length());
				auto short_name_no_prefix_no_suffix = short_name_no_prefix.substr(0, short_name_no_prefix.length() - suffix.length());

				auto snake_name = util::camel_to_snake_case(short_name_no_prefix_no_suffix);
				auto snake_name_reversed = util::reverse_from_separator(snake_name, "_");

				std::string snake_name_no_prefix; // std::string_view

				if (!opt_snake_prefix.empty())
				{
					if (snake_name.starts_with(opt_snake_prefix))
					{
						snake_name_no_prefix = snake_name.substr(opt_snake_prefix.length()+1);
						snake_name_reversed = snake_name_reversed.substr(0, snake_name.length()-opt_snake_prefix.length()-1);
					}
				}

				if constexpr
				(
					std::is_invocable_r_v
					<
						bool, Callback,
						decltype(type_id), decltype(type),
						decltype(short_name), decltype(short_name_no_prefix_no_suffix),
						decltype(snake_name), decltype(snake_name_reversed), decltype(snake_name_no_prefix)
					>
				)
				{
					if (!callback(type_id, type, short_name, short_name_no_prefix_no_suffix, std::move(snake_name), std::move(snake_name_reversed), std::move(snake_name_no_prefix)))
					{
						break;
					}
				}
				else
				{
					callback(type_id, type, short_name, short_name_no_prefix_no_suffix, std::move(snake_name), std::move(snake_name_reversed), std::move(snake_name_no_prefix));
				}
			}
		}
	}

	struct MetaTypeResolutionContext
	{
		// Maps aliases (`std::string`) to statically defined `std::string_view` objects queryable from `entt`.
		using AliasContainer = util::hash_map<std::string_view>; // std::string
		
		template <typename ShortNameFn>
		static std::size_t generate_aliases
		(
			AliasContainer& container_out,
			ShortNameFn&& short_name_fn,

			std::string_view prefix,
			std::string_view suffix,

			bool standard_mapping=true,
			bool reverse_mapping=true,
			
			std::string_view opt_snake_prefix={}
		)
		{
			std::size_t count = 0;

			impl::generate_simplified_type_names
			(
				[&container_out, &short_name_fn, standard_mapping, reverse_mapping, opt_snake_prefix, &count]
				(
					auto&& type_id, auto&& type,
					std::string_view short_name, std::string_view short_name_no_prefix_no_suffix,
					std::string&& snake_name, std::string&& snake_name_reversed, std::string&& snake_name_no_prefix
				)
				{
					if (standard_mapping)
					{
						container_out[std::move(snake_name)] = short_name;

						count++;

						if (!snake_name_no_prefix.empty())
						{
							container_out[std::move(snake_name_no_prefix)] = short_name;

							count++;
						}
					}

					if (reverse_mapping)
					{
						container_out[std::move(snake_name_reversed)] = short_name;

						count++;
					}
				},

				short_name_fn,

				prefix,
				suffix,

				opt_snake_prefix
			);

			return count;
		}

		static MetaTypeResolutionContext generate(bool standard_mapping=true, bool reverse_mapping=true);

		template <typename ShortNameFn, typename CleanUpFn>
		static MetaTypeResolutionContext generate(bool standard_mapping, bool reverse_mapping, ShortNameFn&& short_name_fn, CleanUpFn&& cleanup_fn)
		{
			using namespace engine::literals;

			MetaTypeResolutionContext context;

			// Components:
			auto& components = context.component_aliases;

			generate_aliases
			(
				components,
				short_name_fn,
			
				{},
				"Component",

				standard_mapping,
				reverse_mapping
				//, "entity"
			);

			// Commands:
			auto& commands = context.command_aliases;

			generate_aliases
			(
				commands,
				short_name_fn,
				
				{},
				"Command",

				standard_mapping,
				reverse_mapping,

				// Removes need for `entity` prefix on some commands.
				// (e.g. `entity_thread_resume` becomes `thread_resume`)
				"entity"
			);
		
			// Instructions:
			auto& instructions = context.instruction_aliases;

			generate_aliases
			(
				instructions,
				short_name_fn,
				
				"instructions::",
				{},

				standard_mapping,
				false, // reverse_mapping,

				// Removes need for `entity` prefix on some instructions.
				// (e.g. `entity_thread_instruction` becomes `thread_instruction`)
				"entity"
			);

			// Systems:
			auto& systems = context.system_aliases;

			generate_aliases
			(
				systems,
				short_name_fn,
			
				{},

				"System",

				standard_mapping,
				reverse_mapping
			);

			// Global namespace:
			auto& global_namespace = context.global_namespace;

			// Enumerate every reflected type, checking if the
			// `global namespace` property has been set:
			for (const auto& type_entry : entt::resolve())
			{
				const auto& type = type_entry.second;

				if (type_has_global_namespace_flag(type))
				{
					// Alternative implementation:
					// 
					// NOTE: Unused due to differing identifier from what can be used with `resolve`.
					// 
					// EnTT may be reporting a hash of the original type name,
					// rather than the "custom" (e.g. shortened) type name?
					// 
					//const auto& type_id = type_entry.first;

					const auto type_id = type.id();

					global_namespace.emplace_back(type_id);
				}
			}

			cleanup_fn(context);

			return context;
		}

		template <typename ShortNameFn>
		static MetaTypeResolutionContext generate(bool standard_mapping, bool reverse_mapping, ShortNameFn&& short_name_fn)
		{
			return generate
			(
				standard_mapping, reverse_mapping,
				
				std::forward<ShortNameFn>(short_name_fn),

				[](auto& context)
				{
					return cleanup_generated_aliases(context);
				}
			);
		}

		// Removes unwanted aliases, resolves name conflicts, etc.
		static MetaTypeResolutionContext& cleanup_generated_aliases(MetaTypeResolutionContext& context);

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