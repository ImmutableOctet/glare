#pragma once

namespace engine
{
	struct MetaTypeDescriptorFlags
	{
		bool allow_default_construction : 1 = true;
		bool allow_forwarding_fields_to_constructor : 1 = true;
		bool force_field_assignment : 1 = false;
	};
}