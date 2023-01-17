#pragma once

#include "reflection.hpp"

#include "meta_variable.hpp"
#include "meta_data_member.hpp"
#include "indirect_meta_data_member.hpp"
#include "meta_type_descriptor.hpp"

#include "events.hpp"

namespace engine
{
	template <>
	void reflect<MetaVariable>()
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
	void reflect<MetaDataMember>()
	{
		engine_meta_type<MetaDataMember>()
			.data<&MetaDataMember::type_id>("type_id"_hs)
			.data<&MetaDataMember::data_member_id>("data_member_id"_hs)
			.func<static_cast<MetaAny (MetaDataMember::*)(const MetaAny& instance) const>(&MetaDataMember::get)>("get_value"_hs)
			.func<static_cast<MetaAny (MetaDataMember::*)(Registry& registry, Entity entity) const>(&MetaDataMember::get)>("get_value_from_component"_hs)
		;
	}

	template <>
	void reflect<IndirectMetaDataMember>()
	{
		engine_meta_type<IndirectMetaDataMember>()
			.data<&IndirectMetaDataMember::target>("target"_hs)
			.data<&IndirectMetaDataMember::data_member>("data_member"_hs)
			.func<static_cast<MetaAny (IndirectMetaDataMember::*)(const MetaAny& instance) const>(&IndirectMetaDataMember::get)>("get_value"_hs)
			.func<static_cast<MetaAny (IndirectMetaDataMember::*)(Registry& registry, Entity entity, bool) const>(&IndirectMetaDataMember::get)>("get_value_from_component"_hs)
		;
	}

	template <>
	void reflect<MetaTypeDescriptor>()
	{
		engine_meta_type<MetaTypeDescriptor>()
			.ctor<MetaType>()
			.ctor<MetaType, std::optional<MetaTypeDescriptor::SmallSize>>()

			.ctor<MetaTypeID>()
			.ctor<MetaTypeID, std::optional<MetaTypeDescriptor::SmallSize>>()

			.ctor<MetaType, const util::json&>()
			
			.ctor
			<
				MetaType, std::string_view,
				const MetaAnyParseInstructions&,
				std::string_view,
				std::size_t,
				std::optional<MetaTypeDescriptor::SmallSize>,
				const MetaTypeDescriptorFlags&
			>()

			.ctor
			<
				MetaType, const util::json&,
				const MetaAnyParseInstructions&,
				//std::size_t,
				std::optional<MetaTypeDescriptor::SmallSize>,
				const MetaTypeDescriptorFlags&
			>()

			.data<&MetaTypeDescriptor::type_id>("type_id"_hs)
			.data<&MetaTypeDescriptor::field_names>("field_names"_hs)
			.data<&MetaTypeDescriptor::field_values>("field_values"_hs)
			.data<&MetaTypeDescriptor::constructor_argument_count>("constructor_argument_count"_hs)

			.data<nullptr, &MetaTypeDescriptor::get_type>("type"_hs)

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

	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnComponentCreate,  ComponentEvent);
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnComponentUpdate,  ComponentEvent);
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnComponentDestroy, ComponentEvent);

	template <>
	void reflect<ComponentEvent>()
	{
		engine_meta_type<ComponentEvent>()
			.data<&ComponentEvent::entity>("entity"_hs)
			.data<&ComponentEvent::component>("component"_hs)
			.data<nullptr, &ComponentEvent::type>("type"_hs)
		;

		reflect<OnComponentCreate>();
		reflect<OnComponentUpdate>();
		reflect<OnComponentDestroy>();
	}

	void reflect_meta()
	{
		reflect<MetaVariable>();
		reflect<MetaDataMember>();
		reflect<IndirectMetaDataMember>();
		reflect<MetaTypeDescriptor>();

		reflect<ComponentEvent>();
	}
}