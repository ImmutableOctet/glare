#pragma once

#include "types.hpp"

// TODO: Forward declare `util::json`, etc.
#include <util/json.hpp>

#include <entt/meta/meta.hpp>
//#include <entt/entt.hpp>

#include <optional>

namespace engine
{
	// NOTE: This overload cannot handle non-primitive values.
	// (see overload taking a native-type identifier)
	entt::meta_any resolve_meta_any(const util::json& value);

	entt::meta_any resolve_meta_any(const util::json& value, MetaType type);
	entt::meta_any resolve_meta_any(const util::json& value, MetaTypeID type_id);

	// JSON-shorthand overload for string-to-any resolution function.
	entt::meta_any meta_any_from_string(const util::json& value, bool resolve_symbol, bool strip_quotes=true);

	// JSON-shorthand overload for string-to-any resolution function.
	entt::meta_any meta_any_from_string(const util::json& value);

	// Attempts to resolve a native value from a raw string value, using reflection.
	entt::meta_any meta_any_from_string(std::string_view value, bool resolve_symbol, bool strip_quotes=true);

	// Attempts to resolve a native value from a raw string value, using reflection.
	entt::meta_any meta_any_from_string(std::string_view value);
}