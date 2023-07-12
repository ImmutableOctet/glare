#include "data_member.hpp"

//#include <entt/meta/meta.hpp>

namespace engine
{
    bool data_member_is_read_only(const entt::meta_data& data_member)
    {
        return (data_member.is_const() || (data_member.arity() == 0));
    }

	std::optional<std::pair<entt::id_type, entt::meta_data>>
	get_local_data_member_by_index(const entt::meta_type& type, std::size_t variable_index)
    {
        auto data_range = type.data();
        auto data_it = (data_range.begin() + static_cast<std::ptrdiff_t>(variable_index));

        if (data_it < data_range.end()) // !=
        {
            return *data_it;
        }

        return std::nullopt;
    }

	std::optional<std::pair<entt::id_type, entt::meta_data>>
	get_data_member_by_index(const entt::meta_type& type, std::size_t variable_index, bool recursive)
    {
        if (!recursive)
        {
            return get_local_data_member_by_index(type, variable_index);
        }

        std::optional<std::pair<entt::id_type, entt::meta_data>> output = std::nullopt;
        std::size_t count = 0;

        enumerate_data_members
        (
            type,

            [&output, &count, variable_index](auto&& data_member_id, auto&& data_member)
            {
                if (count != variable_index)
                {
                    count++;

                    return true;
                }

                if (data_member)
                {
                    output = { data_member_id, data_member };
                }
                
                return false;
            }
        );

        return output;
    }

    std::optional<PlayerIndex> resolve_player_index(const MetaAny& instance)
	{
		const auto type = instance.type();

		if (!type)
		{
			return std::nullopt;
		}

		auto [player_index_member_id, player_index_member] = resolve_data_member(type, true, "player_index");

		if (!player_index_member)
		{
			return std::nullopt;
		}

		const auto player_idx = player_index_member.get(instance);

		if (player_idx)
		{
			if (const auto idx_out = player_idx.try_cast<PlayerIndex>())
			{
				return *idx_out;
			}
		}

		return std::nullopt;
	}
}