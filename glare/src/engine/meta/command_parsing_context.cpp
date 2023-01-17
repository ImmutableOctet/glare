#include "command_parsing_context.hpp"

#include "meta.hpp"

namespace engine
{
	// TODO: Move to a different location.
	template <typename Callback>
    inline void generate_simplified_type_names(Callback callback, std::string_view suffix, std::string_view opt_snake_prefix={})
    {
        for (const auto& type_entry : entt::resolve())
        {
            const auto& type_id = type_entry.first;
            const auto& type = type_entry.second;

            const auto& type_info = type.info();
            const auto type_name = type_info.name();

            if (!type_name.ends_with(suffix))
            {
                continue;
            }

            auto short_name = as_short_name(type_name);
            auto short_name_no_suffix = short_name.substr(0, short_name.length() - suffix.length());

            auto snake_name = util::camel_to_snake_case(short_name_no_suffix);
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
                    decltype(short_name), decltype(short_name_no_suffix),
                    decltype(snake_name), decltype(snake_name_reversed), decltype(snake_name_no_prefix)
                >
            )
            {
                if (!callback(type_id, type, short_name, short_name_no_suffix, std::move(snake_name), std::move(snake_name_reversed), std::move(snake_name_no_prefix)))
                {
                    break;
                }
            }
            else
            {
                callback(type_id, type, short_name, short_name_no_suffix, std::move(snake_name), std::move(snake_name_reversed), std::move(snake_name_no_prefix));
            }
        }
    }

	CommandParsingContext CommandParsingContext::generate(bool standard_mapping, bool reverse_mapping, std::string_view suffix)
	{
		CommandParsingContext context;

		generate(context, standard_mapping, reverse_mapping, suffix);

		return context;
	}

	std::size_t CommandParsingContext::generate(CommandParsingContext& context, bool standard_mapping, bool reverse_mapping, std::string_view suffix)
	{
		std::size_t count = 0;

		generate_simplified_type_names
		(
			[&context, standard_mapping, reverse_mapping, &count]
			(
				auto&& type_id, auto&& type,
				std::string_view short_name, std::string_view short_name_no_suffix,
				std::string&& snake_name, std::string&& snake_name_reversed, std::string&& snake_name_no_prefix
			)
			{
				if (standard_mapping)
				{
					context.command_aliases[std::move(snake_name)] = short_name;

					count++;

					if (!snake_name_no_prefix.empty())
					{
						context.command_aliases[std::move(snake_name_no_prefix)] = short_name;

						count++;
					}
				}

				if (reverse_mapping)
				{
					context.command_aliases[std::move(snake_name_reversed)] = short_name;

					count++;
				}
			},

			suffix,

			// Removes need for `entity` prefix on some commands.
			// (e.g. `entity_thread_resume` becomes `thread_resume`)
			"entity"
		);

		return count;
	}

	std::string_view CommandParsingContext::resolve_command_alias(std::string_view command_alias) const
	{
		if (const auto cmd_it = command_aliases.find(command_alias); cmd_it != command_aliases.end())
		{
			const auto& resolved_command_name = cmd_it->second;

			return resolved_command_name;
		}

		return {};
	}

	MetaType CommandParsingContext::get_command_type(std::string_view command_name, bool is_known_alias) const
	{
		if (!is_known_alias)
		{
			const auto command_name_id = hash(command_name);

			if (const auto command_type = resolve(command_name_id))
			{
				return command_type;
			}
		}

		return get_command_type_from_alias(command_name);
	}

	MetaType CommandParsingContext::get_command_type_from_alias(std::string_view command_alias) const
	{
		if (auto resolved_name = resolve_command_alias(command_alias); !resolved_name.empty())
		{
			auto resolved_name_id = hash(resolved_name);

			if (const auto command_type = resolve(resolved_name_id))
			{
				return command_type;
			}
		}

		return {};
	}
}