#pragma once

#include "types.hpp"

#include <util/variant.hpp>
#include <variant>

namespace engine
{
	struct EntityStateTarget
	{
		using SelfTarget = std::monostate;

		struct ParentTarget {};

		struct EntityTarget
		{
			Entity entity;
		};

		struct ChildTarget
		{
			StringHash child_name;

			bool recursive = true;
		};

		struct EntityNameTarget
		{
			StringHash entity_name;
		};

		struct PlayerTarget
		{
			PlayerIndex player_index;
		};

		using TargetType = std::variant
		<
			SelfTarget, // std::monostate,
			ParentTarget,
			EntityTarget,
			EntityNameTarget,
			ChildTarget,
			PlayerTarget
		>;

		enum TargetIndex : std::size_t
		{
			Self        = util::variant_index<TargetType, SelfTarget>(), // 0
			Parent      = util::variant_index<TargetType, ParentTarget>(),
			ExactEntity = util::variant_index<TargetType, EntityTarget>(),
			EntityName  = util::variant_index<TargetType, EntityNameTarget>(),
			Child       = util::variant_index<TargetType, ChildTarget>(),
			Player      = util::variant_index<TargetType, PlayerTarget>(),
		};

		using Type = TargetType;

		TargetType type;

		inline operator TargetType() const
		{
			return type;
		}

		inline std::size_t target_index() const
		{
			return type.index();
		}

		inline bool is_self_targeted() const
		{
			return (target_index() == TargetIndex::Self); // 0 // std::monostate
		}

		inline bool is_parent_target() const
		{
			return (target_index() == TargetIndex::Parent);
		}

		inline bool is_exact_entity_target() const
		{
			return (target_index() == TargetIndex::ExactEntity);
		}

		inline bool is_entity_name_target() const
		{
			return (target_index() == TargetIndex::EntityName);
		}

		inline bool is_child_target() const
		{
			return (target_index() == TargetIndex::Child);
		}

		inline bool is_player_target() const
		{
			return (target_index() == TargetIndex::Player);
		}
	};
}