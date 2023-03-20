#pragma once

#include "types.hpp"

#include <engine/meta/indirect_meta_any.hpp>

#include <util/variant.hpp>

#include <variant>
#include <string_view>
#include <string>
#include <optional>
#include <utility>
#include <tuple>

namespace engine
{
	struct MetaEvaluationContext;
	struct MetaParsingInstructions;

	struct EntityTarget
	{
		public:
			using SelfTarget = std::monostate;

			struct ParentTarget
			{
				inline bool operator==(const ParentTarget&) const noexcept { return true;  } // = default;
				inline bool operator!=(const ParentTarget&) const noexcept { return false; } // = default;
			};

			struct ExactEntityTarget
			{
				Entity entity = null;

				bool operator==(const ExactEntityTarget&) const noexcept = default;
				bool operator!=(const ExactEntityTarget&) const noexcept = default;
			};

			struct ChildTarget
			{
				static ChildTarget from_string(std::string_view child_name, bool recursive);
				static ChildTarget from_string(std::string_view child_name);

				static ChildTarget from_string(const std::string& child_name, bool recursive);
				static ChildTarget from_string(const std::string& child_name);

				StringHash child_name = {};

				bool recursive = true;

				// TODO: Determine if it makes sense to ignore `recursive`. (Probably doesn't)
				bool operator==(const ChildTarget&) const noexcept = default;
				bool operator!=(const ChildTarget&) const noexcept = default;
			};

			struct EntityNameTarget
			{
				static EntityNameTarget from_string(std::string_view entity_name);
				static EntityNameTarget from_string(const std::string& entity_name);

				StringHash entity_name = {};

				bool search_children_first = true;

				bool operator==(const EntityNameTarget&) const noexcept = default;
				bool operator!=(const EntityNameTarget&) const noexcept = default;
			};

			struct PlayerTarget
			{
				PlayerIndex player_index = NO_PLAYER;

				bool operator==(const PlayerTarget&) const noexcept = default;
				bool operator!=(const PlayerTarget&) const noexcept = default;
			};

			struct IndirectTarget
			{
				IndirectMetaAny target_value_expr;

				bool operator==(const IndirectTarget&) const noexcept = default;
				bool operator!=(const IndirectTarget&) const noexcept = default;
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
				IndirectTarget,
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
				Indirect    = util::variant_index<TargetType, IndirectTarget>(),
				Null        = util::variant_index<TargetType, NullTarget>(),
			};

			using Type = TargetType;

			using ParseResult = std::tuple
			<
				TargetType,  // Type
				std::size_t, // Parsing offset into `raw_value`.
				bool         // `is_shorthand` (e.g. "self", "this", "parent")
			>;

			static std::optional<ParseResult> parse_type(std::string_view raw_value, const MetaParsingInstructions* opt_parsing_instructions={});

			static std::optional<EntityTarget> parse(std::string_view raw_value, const MetaParsingInstructions* opt_parsing_instructions={});

			static EntityTarget from_parse_result(const ParseResult& result);
			static std::optional<EntityTarget> from_parse_result(const std::optional<ParseResult>& result);

			template <typename TargetTypeExact>
			static EntityTarget from_target_type(const TargetTypeExact& exact_target_type)
			{
				return EntityTarget { TargetType { exact_target_type } };
			}

			static EntityTarget from_entity(Entity entity);

			static EntityTarget from_string(std::string_view raw_value);
			static EntityTarget from_string(const std::string& raw_value);

			TargetType type = SelfTarget {};

			inline operator TargetType() const
			{
				return type;
			}

			// Alias for `get`.
			Entity resolve(Registry& registry, Entity source=null) const;

			// Attempts to resolve the targeted entity indicated by
			// `type` from `source`, using `registry`.
			// 
			// NOTE: If `source` is `null`, any control path expecting to
			// use or return the initial entity will fail.
			Entity get(Registry& registry, Entity source) const;

			// Attempts to resolve the targeted entity indicated by
			// `type` from `source`, using `registry` and `context`.
			Entity get(Registry& registry, Entity source, const MetaEvaluationContext& context) const;

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

			inline bool is_indirect_target() const
			{
				return (target_index() == TargetIndex::Indirect);
			}

			inline bool is_null_target() const
			{
				return (target_index() == TargetIndex::Null);
			}
		private:
			template <typename ...Args>
			Entity get_impl(Registry& registry, Entity source, Args&&... args) const;
	};

	//static_assert(sizeof(EntityTarget) <= sizeof(double[2]));
}