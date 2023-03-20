#pragma once

#include "entity_descriptor_shared.hpp"

#include <engine/meta/types.hpp>
#include <engine/meta/meta_type_descriptor.hpp>

#include <util/small_vector.hpp>

#include <utility>

namespace engine
{
	class EntityDescriptor;

	// TODO: Rename to something more fitting for the `entity` module.
	struct MetaDescription
	{
		using TypeDescriptors = util::small_vector
		<
			EntityDescriptorShared<MetaTypeDescriptor>,
			16 // 8
		>;

		TypeDescriptors type_definitions;

		const MetaTypeDescriptor* get_definition(const EntityDescriptor& descriptor, MetaType type) const;
		const MetaTypeDescriptor* get_definition(const EntityDescriptor& descriptor, MetaTypeID type_id) const;

		// Non-const forwarding overload.
		inline MetaTypeDescriptor* get_definition(const EntityDescriptor& descriptor, auto&& type_info)
		{
			return const_cast<MetaTypeDescriptor*>
			(
				const_cast<const MetaDescription*>(this)->get_definition
				(
					descriptor,

					std::forward<decltype(type_info)>(type_info)
				)
			);
		}

		bool operator==(const MetaDescription&) const noexcept = default;
		bool operator!=(const MetaDescription&) const noexcept = default;
	};
}