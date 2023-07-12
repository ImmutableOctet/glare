#pragma once

namespace engine
{
	struct MetaEventListenerFlags
	{
		bool events                : 1 = true;
		bool component_creation    : 1 = true;
		bool component_update      : 1 = true;
		bool component_destruction : 1 = true;
	};
}