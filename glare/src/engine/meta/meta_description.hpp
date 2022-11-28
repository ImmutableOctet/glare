#pragma once

#include "types.hpp"
#include "meta_type_descriptor.hpp"

#include <util/small_vector.hpp>

namespace engine
{
	struct MetaDescription
	{
		using TypeDescriptors = util::small_vector<MetaTypeDescriptor, 4>; // 6 // 8

		TypeDescriptors type_definitions;

		const MetaTypeDescriptor* get_definition(MetaType type) const;
		const MetaTypeDescriptor* get_definition(MetaTypeID type_id) const;
	};
}