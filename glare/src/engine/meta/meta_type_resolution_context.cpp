#include "meta_type_resolution_context.hpp"
#include "meta_parsing_instructions.hpp"

#include "meta.hpp"

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
		MetaTypeResolutionContext context;

		generate_aliases
		(
			context.component_aliases,
			
			{},
			"Component",

			standard_mapping,
			reverse_mapping
			//, "entity"
		);

		generate_aliases
		(
			context.command_aliases,
			
			{},
			"Command",

			standard_mapping,
			reverse_mapping,

			// Removes need for `entity` prefix on some commands.
			// (e.g. `entity_thread_resume` becomes `thread_resume`)
			"entity"
		);

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
		instructions["wait"] = instructions["yield"];
		instructions["wake"] = instructions["resume"];

		// May change this to its own instruction type later.
		instructions["step"] = instructions["skip"];

		// Already handled through different instruction type.
		//instructions["sleep"] = instructions["yield"];

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

	MetaType MetaTypeResolutionContext::get_type(std::string_view name, bool resolve_components, bool resolve_commands, bool resolve_instructions) const
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

		return get_type_raw(name);
	}

	MetaTypeID MetaTypeResolutionContext::get_type_id(std::string_view name, bool resolve_components, bool resolve_commands, bool resolve_instructions) const
	{
		if (auto type = get_type(name, resolve_components, resolve_commands, resolve_instructions))
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
			instructions.resolve_instruction_aliases
		);
	}

	MetaTypeID MetaTypeResolutionContext::get_type_id(std::string_view name, const MetaParsingInstructions& instructions) const
	{
		return get_type_id
		(
			name,

			instructions.resolve_component_aliases,
			instructions.resolve_command_aliases,
			instructions.resolve_instruction_aliases
		);
	}
}