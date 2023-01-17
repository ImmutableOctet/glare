#pragma once

#include "types.hpp"

#include <util/variant.hpp>

#include <variant>
#include <string_view>
#include <string>
#include <optional>
#include <utility>
#include <tuple>

namespace engine
{
	struct EntityTarget
	{
		using SelfTarget = std::monostate;

		struct ParentTarget
		{
			inline bool operator==(const ParentTarget&) const noexcept { return true;  } // = default;
			inline bool operator!=(const ParentTarget&) const noexcept { return false; } // = default;
		};

		struct ExactEntityTarget
		{
			Entity entity;

			bool operator==(const ExactEntityTarget&) const noexcept = default;
			bool operator!=(const ExactEntityTarget&) const noexcept = default;
		};

		struct ChildTarget
		{
			StringHash child_name;

			bool recursive = true;

			// TODO: Determine if it makes sense to ignore `recursive`. (Probably doesn't)
			bool operator==(const ChildTarget&) const noexcept = default;
			bool operator!=(const ChildTarget&) const noexcept = default;
		};

		struct EntityNameTarget
		{
			StringHash entity_name;

			bool operator==(const EntityNameTarget&) const noexcept = default;
			bool operator!=(const EntityNameTarget&) const noexcept = default;
		};

		struct PlayerTarget
		{
			PlayerIndex player_index;

			bool operator==(const PlayerTarget&) const noexcept = default;
			bool operator!=(const PlayerTarget&) const noexcept = default;
		};

		struct NullTarget
		{
			inline bool operator==(const NullTarget&) const noexcept { return true;  } // = default;
			inline bool operator!=(const NullTarget&) const noexcept { return false; } // = default;
		};

		using TargetType = std::variant
		<
			SelfTarget, // std::monostate,
			ParentTarget,
			ExactEntityTarget,
			EntityNameTarget,
			ChildTarget,
			PlayerTarget,
			NullTarget
		>;

		enum TargetIndex : std::size_t
		{
			Self        = util::variant_index<TargetType, SelfTarget>(), // 0
			Parent      = util::variant_index<TargetType, ParentTarget>(),
			ExactEntity = util::variant_index<TargetType, ExactEntityTarget>(),
			EntityName  = util::variant_index<TargetType, EntityNameTarget>(),
			Child       = util::variant_index<TargetType, ChildTarget>(),
			Player      = util::variant_index<TargetType, PlayerTarget>(),
			Null        = util::variant_index<TargetType, NullTarget>(),
		};

		using Type = TargetType;

		using ParseResult = std::tuple
		<
			TargetType,  // Type
			std::size_t, // Parsing offset into `raw_value`.
			bool         // `is_shorthand` (e.g. "self", "this", "parent")
		>;

		static std::optional<ParseResult> parse_type(std::string_view raw_value);
		static std::optional<ParseResult> parse_type(const std::string& raw_value);

		template <typename StringType>
		static std::optional<EntityTarget> parse(StringType&& raw_value)
		{
			auto result = parse_type(std::forward<StringType>(raw_value));

			if (!result)
			{
				return std::nullopt;
			}

			return EntityTarget { std::move(std::get<0>(*result)) };
		}

		TargetType type = SelfTarget {};

		inline operator TargetType() const
		{
			return type;
		}

		// Attempts to resolve the targeted entity indicated by
		// `type` from `source`, using `registry`.
		// 
		// NOTE: If `source` is `null`, any control path expecting to
		// use or return the initial entity will fail.
		Entity resolve(Registry& registry, Entity source=null) const;

		// Alias for `resolve`. (Useful for generic programming contexts)
		inline Entity get(Registry& registry, Entity source=null) const
		{
			return resolve(registry, source);
		}

		bool operator==(const EntityTarget&) const noexcept = default;
		bool operator!=(const EntityTarget&) const noexcept = default;

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

		inline bool is_null_target() const
		{
			return (target_index() == TargetIndex::Null);
		}
	};
}