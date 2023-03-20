#include "entity_context.hpp"

namespace engine
{
	std::size_t EntityContext::set_missing_variables(const EntityContext& existing_context)
	{
		existing_context.variables.enumerate_variables
		(
			[this](MetaSymbolID variable_name, const MetaAny& variable_value)
			{
				this->variables.set_missing(variable_name, MetaAny { variable_value });
			}
		);

		return 0;
	}
}