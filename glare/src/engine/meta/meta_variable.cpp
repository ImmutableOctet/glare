#include "meta_variable.hpp"
#include "meta.hpp"
#include "serial.hpp"

namespace engine
{
	MetaVariable::MetaVariable(MetaSymbolID name, entt::meta_any&& value)
		: name(name), value(std::move(value)) {}

	MetaVariable::MetaVariable(MetaSymbolID name, const util::json& value, const Instructions& instructions)
		: MetaVariable(name, resolve_meta_any(value, instructions))
	{}

	MetaVariable::MetaVariable(MetaSymbolID name, const util::json& value, MetaType type, const Instructions& instructions)
		: MetaVariable(name, resolve_meta_any(value, type, instructions))
	{}

	MetaVariable::MetaVariable(MetaSymbolID name, const util::json& value, MetaTypeID type_id, const Instructions& instructions)
		: MetaVariable(name, resolve_meta_any(value, type_id, instructions))
	{}

	MetaVariable::MetaVariable(std::string_view name, const util::json& value, const Instructions& instructions)
		: MetaVariable(hash(name), value, instructions) {}

	MetaVariable::MetaVariable(std::string_view name, const util::json& value, MetaType type, const Instructions& instructions)
		: MetaVariable(hash(name), value, type, instructions)
	{}

	MetaVariable::MetaVariable(std::string_view name, const util::json& value, MetaTypeID type_id, const Instructions& instructions)
		: MetaVariable(hash(name), value, type_id, instructions)
	{}
}