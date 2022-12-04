#pragma once

#include "types.hpp"
#include "meta_data_member.hpp"

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
	// NOTE: This overload cannot handle non-primitive values.
	// (see overload taking a native-type identifier)
	entt::meta_any resolve_meta_any(const util::json& value);

	entt::meta_any resolve_meta_any(const util::json& value, MetaType type);
	entt::meta_any resolve_meta_any(const util::json& value, MetaTypeID type_id);

	// JSON-shorthand overload for string-to-any resolution function.
	entt::meta_any meta_any_from_string(const util::json& value, bool resolve_symbol, bool strip_quotes=true, bool fallback_to_string=true);

	// JSON-shorthand overload for string-to-any resolution function.
	entt::meta_any meta_any_from_string(const util::json& value);

	// Attempts to resolve a native value from a raw string value, using reflection.
	entt::meta_any meta_any_from_string(std::string_view value, bool resolve_symbol, bool strip_quotes=true, bool fallback_to_string=true);

	// Attempts to resolve a native value from a raw string value, using reflection.
	entt::meta_any meta_any_from_string(std::string_view value);

	// Attempts to resolve a `MetaDataMember` from `value`.
	std::optional<MetaDataMember> meta_data_member_from_string(const std::string& value); // std::string_view

	// Attempts to resolve a `MetaDataMember` from `value`.
	std::optional<MetaDataMember> meta_data_member_from_string(std::string_view value);

	std::tuple<std::string_view, std::string_view> parse_data_member_reference(const std::string& value); // std::string_view
}