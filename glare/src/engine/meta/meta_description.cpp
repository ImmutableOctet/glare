#include "meta_description.hpp"
#include "meta.hpp"

namespace engine
{
	const MetaTypeDescriptor* MetaDescription::get_definition(MetaType type) const
	{
		for (const auto& type_desc : type_definitions)
		{
			if (type_desc.type == type)
			{
				return &type_desc;
			}
		}

		return nullptr;
	}

	const MetaTypeDescriptor* MetaDescription::get_definition(MetaTypeID type_id) const
	{
		return get_definition(resolve(type_id));
	}
}