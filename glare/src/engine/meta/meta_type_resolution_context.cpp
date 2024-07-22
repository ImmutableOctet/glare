#include "meta_type_resolution_context.hpp"
#include "meta_parsing_instructions.hpp"

#include "hash.hpp"
#include "short_name.hpp"
#include "indirection.hpp"
#include "data_member.hpp"

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

	MetaTypeResolutionContext MetaTypeResolutionContext::generate(bool standard_mapping, bool reverse_mapping)
	{
		return generate(standard_mapping, reverse_mapping, [](auto&& type_name) { return as_short_name(std::forward<decltype(type_name)>(type_name)); });
	}

	MetaTypeResolutionContext& MetaTypeResolutionContext::cleanup_generated_aliases(MetaTypeResolutionContext& context)
	{
		// Components:
		auto& components = context.component_aliases;

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

		// Removed due to conflict with extension member-function of `Entity`.
		commands.erase("set_parent");
		
		// Instructions:
		auto& instructions = context.instruction_aliases;

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

		// Ensure we don't have a name conflict between the built-in
		// `entity` command (i.e. 'target' syntax) and the `EntitySystem` type.
		systems.erase("entity");

		systems["debug"] = systems["DebugListener"];

		// Services:
		auto& services = context.service_aliases;

		services["service"] = "Service";
		services["world"] = "World";

		return context;
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
		auto get_type_impl = [&aliases, is_known_alias](auto&& name)
		{
			if (!is_known_alias)
			{
				if (auto type = get_type_raw(std::forward<decltype(name)>(name)))
				{
					return type;
				}
			}

			return get_type_from_alias(aliases, std::forward<decltype(name)>(name));
		};

		if (auto type = get_type_impl(name))
		{
			return type;
		}

		constexpr auto separator = std::string_view { "/" };

		if (name.contains(separator))
		{
			auto type = MetaType {};

			auto processed_name = std::string {};

			util::reverse_split
			(
				name, separator,

				[&](auto&& name_path_segment)
				{
					if (!processed_name.empty())
					{
						processed_name.insert(processed_name.begin(), '_');
					}

					processed_name.insert_range(processed_name.begin(), name_path_segment);

					type = get_type_impl(processed_name);

					if (type)
					{
						return false;
					}

					return true;
				}
			);

			if (type)
			{
				return type;
			}
		}

		return {};
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