#pragma once

#include <engine/reflection.hpp>

#include "meta_variable.hpp"
#include "meta_type_descriptor.hpp"

namespace engine
{
	template <>
	inline void reflect<MetaVariable>()
	{
		engine_meta_type<MetaVariable>()
			.ctor<MetaSymbolID, entt::meta_any&&>()
			
			.ctor<MetaSymbolID, const util::json&>()
			.ctor<MetaSymbolID, const util::json&, MetaType>()
			.ctor<MetaSymbolID, const util::json&, MetaTypeID>()

			.ctor<std::string_view, const util::json&>()
			.ctor<std::string_view, const util::json&, MetaType>()
			.ctor<std::string_view, const util::json&, MetaTypeID>()

			.data<&MetaVariable::name>("name"_hs)
			.data<&MetaVariable::value>("value"_hs)

			.func<&MetaVariable::has_name>("has_name"_hs)
		;
	}

	template <>
	inline void reflect<MetaTypeDescriptor>()
	{
		engine_meta_type<MetaTypeDescriptor>()
			.ctor<MetaType>()
			.ctor<MetaType, std::optional<MetaTypeDescriptor::SmallSize>>()

			.ctor<MetaTypeID>()
			.ctor<MetaTypeID, std::optional<MetaTypeDescriptor::SmallSize>>()

			.ctor<MetaType, const util::json&>()
			.ctor<MetaType, const util::json&, std::optional<MetaTypeDescriptor::SmallSize>>()

			.data<&MetaTypeDescriptor::type>("type"_hs)
			.data<&MetaTypeDescriptor::field_names>("field_names"_hs)
			.data<&MetaTypeDescriptor::field_values>("field_values"_hs)
			.data<&MetaTypeDescriptor::constructor_argument_count>("constructor_argument_count"_hs)

			.data<nullptr, &MetaTypeDescriptor::can_default_construct>("can_default_construct"_hs)
			.data<nullptr, &MetaTypeDescriptor::can_forward_fields_to_constructor>("can_forward_fields_to_constructor"_hs)
			.data<nullptr, &MetaTypeDescriptor::forces_field_assignment>("forces_field_assignment"_hs)

			.data<nullptr, &MetaTypeDescriptor::data>("data"_hs)
			.data<nullptr, &MetaTypeDescriptor::size>("size"_hs)

			.data<nullptr, static_cast<bool (MetaTypeDescriptor::*)() const>(&MetaTypeDescriptor::has_nested_descriptor)>("has_nested_descriptor"_hs)

			.func<static_cast<const MetaAny* (MetaTypeDescriptor::*)(MetaSymbolID) const>(&MetaTypeDescriptor::get_variable)>("get_variable"_hs)
			//.func<&MetaTypeDescriptor::set_variable>("set_variable"_hs)
			//.func<&MetaTypeDescriptor::set_variables>("set_variables"_hs)

			// NOTE: `operator()` used here due to conflict when reflecting boolean parameter `allow_recursion`.
			.func<&MetaTypeDescriptor::operator()>("instance"_hs) // &MetaTypeDescriptor::instance
		;
	}

	inline void reflect_meta()
	{
		reflect<MetaVariable>();
		reflect<MetaTypeDescriptor>();
	}
}