#include "meta_type_resolution_context.hpp"
#include "meta_parsing_instructions.hpp"

#include "hash.hpp"
#include "short_name.hpp"
#include "indirection.hpp"
#include "data_member.hpp"

#include <util/string.hpp>
//#include <util/parse.hpp>

#include <algorithm>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	static MetaType get_type_raw(std::string_view name)
	{
		const auto name_id = hash(name);

		return resolve(name_id);
	}

	// TODO: Move to a different location.
	template <typename Callback>
    static void generate_simplified_type_names
	(
		Callback&& callback,

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

            auto short_name = as_short_name(type_name);

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

	MetaTypeResolutionContext MetaTypeResolutionContext::generate(bool standard_mapping, bool reverse_mapping)
	{
		using namespace engine::literals;

		MetaTypeResolutionContext context;

		// Components:
		auto& components = context.component_aliases;

		generate_aliases
		(
			components,
			
			{},
			"Component",

			standard_mapping,
			reverse_mapping
			//, "entity"
		);

		// Removed due to conflict with `Entity::state` property.
		components.erase("state");

		// Removed due to conflict with `EntityTarget` syntax.
		components.erase("player");

		// Removed due to conflict with `Entity::rotate` property.
		components.erase("rotate");

		// Removed due to conflict with `Entity::direction` property.
		components.erase("direction");

		// Removed due to conflict with `AnimationSystem`.
		components.erase("animation");

		// Commands:
		auto& commands = context.command_aliases;

		generate_aliases
		(
			commands,
			
			{},
			"Command",

			standard_mapping,
			reverse_mapping,

			// Removes need for `entity` prefix on some commands.
			// (e.g. `entity_thread_resume` becomes `thread_resume`)
			"entity"
		);

		// Removed due to conflict with extension member-function of `Entity`.
		commands.erase("set_parent");
		
		// Instructions:
		auto& instructions = context.instruction_aliases;

		generate_aliases
		(
			instructions,
			
			"instructions::",
			{},

			standard_mapping,
			false, // reverse_mapping,

			// Removes need for `entity` prefix on some instructions.
			// (e.g. `entity_thread_instruction` becomes `thread_instruction`)
			"entity"
		);

		instructions["terminate"] = instructions["stop"];
		instructions["wait"]      = instructions["yield"];
		instructions["wake"]      = instructions["resume"];
		instructions["event"]     = instructions["event_capture"];
		instructions["capture"]   = instructions["event_capture"];

		// May change this to its own instruction type later.
		instructions["step"]      = instructions["skip"];

		// Already handled through different instruction type.
		//instructions["sleep"]   = instructions["yield"];

		// Systems:
		auto& systems = context.system_aliases;

		generate_aliases
		(
			systems,
			
			{},

			"System",

			standard_mapping,
			reverse_mapping
		);

		// Ensure we don't have a name conflict between the built-in
		// `entity` command (i.e. 'target' syntax) and the `EntitySystem` type.
		systems.erase("entity");

		systems["debug"] = systems["DebugListener"];

		// Services:
		auto& services = context.service_aliases;

		services["service"] = "Service";
		services["world"] = "World";

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

		return context;
	}

	std::size_t MetaTypeResolutionContext::generate_aliases
	(
		AliasContainer& container_out,
		std::string_view prefix,
		std::string_view suffix,
		bool standard_mapping, bool reverse_mapping,
		std::string_view opt_snake_prefix
	)
	{
		std::size_t count = 0;

		generate_simplified_type_names
		(
			[&container_out, standard_mapping, reverse_mapping, opt_snake_prefix, &count]
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

			prefix,
			suffix,

			opt_snake_prefix
		);

		return count;
	}

	std::string_view MetaTypeResolutionContext::resolve_alias(const AliasContainer& container, std::string_view alias)
	{
		if (const auto it = container.find(alias); it != container.end())
		{
			const auto& resolved_name = it->second;

			return resolved_name;
		}

		return {};
	}

	MetaType MetaTypeResolutionContext::get_type(const AliasContainer& aliases, std::string_view name, bool is_known_alias)
	{
		if (!is_known_alias)
		{
			if (auto type = get_type_raw(name))
			{
				return type;
			}
		}

		return get_type_from_alias(aliases, name);
	}

	MetaType MetaTypeResolutionContext::get_type_from_alias(const AliasContainer& alias_container, std::string_view alias)
	{
		if (auto resolved_name = resolve_alias(alias_container, alias); !resolved_name.empty())
		{
			auto resolved_name_id = hash(resolved_name);

			if (const auto type = resolve(resolved_name_id))
			{
				return type;
			}
		}

		return {};
	}

	MetaType MetaTypeResolutionContext::get_type
	(
		std::string_view name,
		
		bool resolve_components,
		bool resolve_commands,
		bool resolve_instructions,
		bool resolve_systems,
		bool resolve_services
	) const
	{
		if (resolve_instructions)
		{
			if (auto as_instruction = get_instruction_type(name, true))
			{
				return as_instruction;
			}
		}

		if (resolve_commands)
		{
			if (auto as_command = get_command_type(name, true))
			{
				return as_command;
			}
		}

		if (resolve_components)
		{
			if (auto as_component = get_component_type(name, true))
			{
				return as_component;
			}
		}

		if (resolve_systems)
		{
			if (auto as_system = get_system_type(name, true))
			{
				return as_system;
			}
		}

		if (resolve_services)
		{
			if (auto as_service = get_service_type(name, true))
			{
				return as_service;
			}
		}

		return get_type_raw(name);
	}

	MetaTypeID MetaTypeResolutionContext::get_type_id
	(
		std::string_view name,

		bool resolve_components,
		bool resolve_commands,
		bool resolve_instructions,
		bool resolve_systems,
		bool resolve_services
	) const
	{
		auto type = get_type
		(
			name,
			
			resolve_components,
			resolve_commands,
			resolve_instructions,
			resolve_systems,
			resolve_services
		);

		if (type)
		{
			return type.id();
		}

		return {};
	}

	MetaType MetaTypeResolutionContext::get_type(std::string_view name, const MetaParsingInstructions& instructions) const
	{
		return get_type
		(
			name,
			
			instructions.resolve_component_aliases,
			instructions.resolve_command_aliases,
			instructions.resolve_instruction_aliases,
			instructions.resolve_system_references,
			instructions.resolve_service_references
		);
	}

	MetaTypeID MetaTypeResolutionContext::get_type_id(std::string_view name, const MetaParsingInstructions& instructions) const
	{
		return get_type_id
		(
			name,

			instructions.resolve_component_aliases,
			instructions.resolve_command_aliases,
			instructions.resolve_instruction_aliases,
			instructions.resolve_system_references,
			instructions.resolve_service_references
		);
	}

	bool MetaTypeResolutionContext::type_in_global_namespace(const MetaType& type) const
	{
		if (!type)
		{
			return false;
		}

		return type_in_global_namespace(type.id());
	}

	bool MetaTypeResolutionContext::type_in_global_namespace(MetaTypeID type_id) const
	{
		if (!type_id)
		{
			return false;
		}

		if (auto it = std::find(global_namespace.begin(), global_namespace.end(), type_id); it != global_namespace.end())
		{
			return true;
		}

		return false;
	}

	std::tuple<MetaType, MetaFunction> MetaTypeResolutionContext::resolve_global_function(MetaFunctionID function_id) const
	{
		for (const auto& type_id : global_namespace)
		{
			auto type = resolve(type_id);

			assert(type);

			if (type)
			{
				if (auto fn = type.func(function_id))
				{
					return { std::move(type), std::move(fn) };
				}
			}
		}

		return {};
	}

	std::tuple<MetaType, entt::meta_data> MetaTypeResolutionContext::resolve_global_data_member(MetaSymbolID member_id) const
	{
		for (const auto& type_id : global_namespace)
		{
			auto type = resolve(type_id);

			assert(type);

			if (type)
			{
				if (auto data_member = resolve_data_member_by_id(type, true, member_id)) // type.data(member_id)
				{
					return { std::move(type), std::move(data_member) };
				}
			}
		}

		return {};
	}

	std::tuple<MetaType, entt::meta_prop> MetaTypeResolutionContext::resolve_global_property(MetaSymbolID property_id) const
	{
		for (const auto& type_id : global_namespace)
		{
			auto type = resolve(type_id);

			assert(type);

			if (type)
			{
				if (auto global_property = type.prop(property_id))
				{
					return { std::move(type), std::move(global_property) };
				}
			}
		}

		return {};
	}
}