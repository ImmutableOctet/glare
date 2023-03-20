#include "meta_description.hpp"
#include "entity_descriptor.hpp"

//#include <engine/meta/meta.hpp>

namespace engine
{
	const MetaTypeDescriptor* MetaDescription::get_definition(const EntityDescriptor& descriptor, MetaType type) const
	{
		return get_definition(descriptor, type.id());
	}

	const MetaTypeDescriptor* MetaDescription::get_definition(const EntityDescriptor& descriptor, MetaTypeID type_id) const
	{
		for (const auto& type_desc_entry : type_definitions)
		{
			const auto& type_desc = type_desc_entry.get(descriptor);

			if (type_desc.get_type_id() == type_id)
			{
				return &type_desc;
			}
		}

		return nullptr;
	}
}