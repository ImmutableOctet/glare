#pragma once

#include "types.hpp"
#include "meta_data_member.hpp"
#include "indirect_meta_data_member.hpp"

#include <engine/entity/entity_target.hpp>

// TODO: Forward declare `util::json`, etc.
#include <util/json.hpp>

#include <entt/meta/meta.hpp>
//#include <entt/entt.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <tuple>

namespace engine
{
	// JSON-shorthand overload for string-to-any resolution function.
	entt::meta_any meta_any_from_string(const util::json& value, const MetaAnyParseInstructions& instructions={});

	// Attempts to resolve a native value from a raw string value, using reflection.
	entt::meta_any meta_any_from_string(std::string_view value, const MetaAnyParseInstructions& instructions={});

	// Attempts to resolve the value indicated by `string_reference` as a string.
	// 
	// If the value is a string, `string_callback` will be executed.
	// If the value is not a string, `non_string_callback` will be called instead.
	// 
	// NOTE: Although the return value of `string_callback` is ignored, the return-value of
	// `non_string_callback` is used to determine if the `entt::meta_any` instance
	// retrieved should be returned back to the initial caller.
	template <typename StringCallback, typename NonStringCallback>
	inline entt::meta_any peek_string_value(std::string_view string_reference, StringCallback&& string_callback, NonStringCallback&& non_string_callback, const MetaAnyParseInstructions& instructions={}) // { .fallback_to_string=true }
	{
		const auto resolved_value = meta_any_from_string(string_reference, instructions);

		if (!resolved_value)
		{
			return {};
		}

		if (try_string_value(resolved_value, string_callback))
		{
			if constexpr (std::is_invocable_r_v<bool, NonStringCallback, decltype(resolved_value)>)
			{
				if (!non_string_callback(resolved_value))
				{
					return {};
				}
			}
			else
			{
				non_string_callback(resolved_value);
			}
		}

		return resolved_value;
	}

	// Convenience overload for `peek_string_value` without the need to specify a non-string callback.
	template <typename Callback>
	inline bool peek_string_value(std::string_view string_reference, Callback&& callback, const MetaAnyParseInstructions& instructions={}) // { .fallback_to_string=true }
	{
		auto result = peek_string_value
		(
			string_reference,
			std::forward<Callback>(callback),
			
			[](const entt::meta_any& non_string_value)
			{
				return false;
			},

			instructions
		);

		return static_cast<bool>(result);
	}

	entt::meta_any resolve_meta_any(const util::json& value, MetaTypeID type_id, const MetaAnyParseInstructions& instructions={});
	entt::meta_any resolve_meta_any(const util::json& value, MetaType type, const MetaAnyParseInstructions& instructions={});

	// NOTE: This overload cannot handle non-primitive values.
	// (see overload taking a native-type identifier)
	entt::meta_any resolve_meta_any(const util::json& value, const MetaAnyParseInstructions& instructions={});

	// Attempts to resolve a `MetaDataMember` from `value`.
	// NOTE: Does not support indirection. (i.e. other entities from the initial source)
	std::optional<MetaDataMember> meta_data_member_from_string(const std::string& value);

	// Attempts to resolve a `MetaDataMember` from `value`.
	// NOTE: Does not support indirection. (i.e. other entities from the initial source)
	std::optional<MetaDataMember> meta_data_member_from_string(std::string_view value);

	// Attempts to resolve `type_name` and `data_member_name`, then returns a
	// `MetaDataMember` instance using the corresponding reflection data.
	std::optional<MetaDataMember> process_meta_data_member(std::string_view type_name, std::string_view data_member_name);

	// Attempts to resolve an `IndirectMetaDataMember` from `value`.
	std::optional<IndirectMetaDataMember> indirect_meta_data_member_from_string(std::string_view value, EntityTarget target = { EntityTarget::SelfTarget{} });

	// Attempts to resolve an `IndirectMetaDataMember` from `value`.
	std::optional<IndirectMetaDataMember> indirect_meta_data_member_from_string(const std::string& value, EntityTarget target = { EntityTarget::SelfTarget{} });

	std::tuple<std::string_view, std::string_view> parse_data_member_reference(const std::string& value); // std::string_view

	EntityTarget::TargetType parse_target_type(const util::json& target_data);
}