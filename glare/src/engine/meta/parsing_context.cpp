#include "parsing_context.hpp"

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

	ParsingContext ParsingContext::generate(bool standard_mapping, bool reverse_mapping)
	{
		ParsingContext context;

		generate_aliases
		(
			context.command_aliases,
			
			"Command",

			standard_mapping, reverse_mapping,

			// Removes need for `entity` prefix on some commands.
			// (e.g. `entity_thread_resume` becomes `thread_resume`)
			"entity"
		);

		generate_aliases
		(
			context.component_aliases,
			
			"Component",

			standard_mapping, reverse_mapping
			//, "entity"
		);

		return context;
	}

	std::size_t ParsingContext::generate_aliases(AliasContainer& container_out, std::string_view suffix, bool standard_mapping, bool reverse_mapping, std::string_view opt_snake_prefix)
	{
		std::size_t count = 0;

		generate_simplified_type_names
		(
			[&container_out, standard_mapping, reverse_mapping, opt_snake_prefix, &count]
			(
				auto&& type_id, auto&& type,
				std::string_view short_name, std::string_view short_name_no_suffix,
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

			suffix,

			opt_snake_prefix
		);

		return count;
	}

	std::string_view ParsingContext::resolve_alias(const AliasContainer& container, std::string_view alias)
	{
		if (const auto it = container.find(alias); it != container.end())
		{
			const auto& resolved_name = it->second;

			return resolved_name;
		}

		return {};
	}

	MetaType ParsingContext::get_type(const AliasContainer& aliases, std::string_view name, bool is_known_alias)
	{
		if (!is_known_alias)
		{
			const auto name_id = hash(name);

			if (const auto type = resolve(name_id))
			{
				return type;
			}
		}

		return get_type_from_alias(aliases, name);
	}

	MetaType ParsingContext::get_type_from_alias(const AliasContainer& alias_container, std::string_view alias)
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
}