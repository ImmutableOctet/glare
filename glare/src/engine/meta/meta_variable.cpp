#include "meta_variable.hpp"
#include "meta.hpp"
#include "serial.hpp"

namespace engine
{
	MetaVariable::MetaVariable(MetaSymbolID name, entt::meta_any&& value)
		: name(name), value(std::move(value)) {}

	MetaVariable::MetaVariable(MetaSymbolID name, const util::json& value)
		: MetaVariable(name, resolve_meta_any(value))
	{}

	MetaVariable::MetaVariable(MetaSymbolID name, const util::json& value, MetaType type)
		: MetaVariable(name, resolve_meta_any(value, type))
	{}

	MetaVariable::MetaVariable(MetaSymbolID name, const util::json& value, MetaTypeID type_id)
		: MetaVariable(name, resolve_meta_any(value, type_id))
	{}

	MetaVariable::MetaVariable(std::string_view name, const util::json& value)
		: MetaVariable(hash(name), value) {}

	MetaVariable::MetaVariable(std::string_view name, const util::json& value, MetaType type)
		: MetaVariable(hash(name), value, type)
	{}

	MetaVariable::MetaVariable(std::string_view name, const util::json& value, MetaTypeID type_id)
		: MetaVariable(hash(name), value, type_id)
	{}
}