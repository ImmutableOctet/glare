#include "meta.hpp"

namespace engine
{
	std::optional<PlayerIndex> resolve_player_index(const MetaAny& instance)
	{
		using namespace entt::literals;

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