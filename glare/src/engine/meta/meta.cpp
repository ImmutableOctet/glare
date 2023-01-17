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

	bool meta_any_is_string(const entt::meta_any& value)
	{
		if (!value)
		{
			return false;
		}

		auto type = value.type();

		if (!type)
		{
			return false;
		}

		const auto type_id = type.id();

		return
		(
			(type_id == entt::type_id<std::string>().hash())
			||
			(type_id == entt::type_id<std::string_view>().hash())
		);
	}

	std::optional<StringHash> meta_any_to_string_hash(const entt::meta_any& value)
	{
		if (!value)
		{
			return std::nullopt;
		}

		std::optional<StringHash> hash_out = std::nullopt;

		if
		(
			try_string_value
			(
				value, [&hash_out](const auto& str_value)
				{
					hash_out = hash(str_value);
				}
			)
		)
		{
			return hash_out;
		}

		if
		(
			try_value<StringHash>
			(
				value,
				[&hash_out](const auto& hash)
				{
					hash_out = hash;
				}
			)
		)
		{
			return hash_out;
		}

		if
		(
			try_value<std::optional<StringHash>>
			(
				value,
				[&hash_out](const auto& opt_hash)
				{
					hash_out = opt_hash;
				}
			)
		)
		{
			return hash_out;
		}

		return hash_out; // std::nullopt;
	}

	bool meta_any_string_compare(const entt::meta_any& left, const entt::meta_any& right)
	{
		auto left_hash = meta_any_to_string_hash(left);

		if (!left_hash)
		{
			return false;
		}

		auto right_hash = meta_any_to_string_hash(right);

		if (!right_hash)
		{
			return false;
		}

		return (left_hash == right_hash);
	}
}