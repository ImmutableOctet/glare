#include "meta_variable.hpp"
#include "meta.hpp"
#include "serial.hpp"

namespace engine
{
	MetaVariable::MetaVariable(MetaSymbolID name, entt::meta_any&& value)
		: name(name), value(std::move(value)) {}

	MetaVariable::MetaVariable(std::string_view name, entt::meta_any&& value)
		: name(hash(name)), value(std::move(value)) {}

	MetaVariable::MetaVariable(MetaSymbolID name, const util::json& value, const Instructions& instructions, const ParsingContext* opt_parsing_context)
		: MetaVariable(name, resolve_meta_any(value, instructions, opt_parsing_context))
	{}

	MetaVariable::MetaVariable(MetaSymbolID name, const util::json& value, MetaType type, const Instructions& instructions, const ParsingContext* opt_parsing_context)
		: MetaVariable(name, resolve_meta_any(value, type, instructions, opt_parsing_context))
	{}

	MetaVariable::MetaVariable(MetaSymbolID name, const util::json& value, MetaTypeID type_id, const Instructions& instructions, const ParsingContext* opt_parsing_context)
		: MetaVariable(name, resolve_meta_any(value, type_id, instructions, opt_parsing_context))
	{}

	MetaVariable::MetaVariable(std::string_view name, const util::json& value, const Instructions& instructions, const ParsingContext* opt_parsing_context)
		: MetaVariable(hash(name), value, instructions, opt_parsing_context) {}

	MetaVariable::MetaVariable(std::string_view name, const util::json& value, MetaType type, const Instructions& instructions, const ParsingContext* opt_parsing_context)
		: MetaVariable(hash(name), value, type, instructions, opt_parsing_context)
	{}

	MetaVariable::MetaVariable(std::string_view name, const util::json& value, MetaTypeID type_id, const Instructions& instructions, const ParsingContext* opt_parsing_context)
		: MetaVariable(hash(name), value, type_id, instructions, opt_parsing_context)
	{}
}