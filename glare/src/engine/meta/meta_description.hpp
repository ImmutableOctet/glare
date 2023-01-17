#pragma once

#include "types.hpp"
#include "meta_type_descriptor.hpp"

#include <util/small_vector.hpp>

#include <utility>

namespace engine
{
	struct MetaDescription
	{
		using TypeDescriptors = util::small_vector<MetaTypeDescriptor, 4>; // 6 // 8

		TypeDescriptors type_definitions;

		const MetaTypeDescriptor* get_definition(MetaType type) const;
		const MetaTypeDescriptor* get_definition(MetaTypeID type_id) const;

		// Non-const forwarding overload.
		inline MetaTypeDescriptor* get_definition(auto&& type_info)
		{
			return const_cast<MetaTypeDescriptor*>
			(
				const_cast<const MetaDescription*>(this)->get_definition
				(
					std::forward<decltype(type_info)>(type_info)
				)
			);
		}
	};
}