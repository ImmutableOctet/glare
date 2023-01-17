#include "meta_description.hpp"
#include "meta.hpp"

namespace engine
{
	const MetaTypeDescriptor* MetaDescription::get_definition(MetaType type) const
	{
		return get_definition(type.id());
	}

	const MetaTypeDescriptor* MetaDescription::get_definition(MetaTypeID type_id) const
	{
		for (const auto& type_desc : type_definitions)
		{
			if (type_desc.type_id == type_id)
			{
				return &type_desc;
			}
		}

		return nullptr;
	}
}