#pragma once

#include "reflection.hpp"

#include "meta_any_wrapper.hpp"
#include "meta_variable.hpp"
#include "meta_variable_scope.hpp"
#include "meta_variable_target.hpp"
#include "meta_variable_context.hpp"
#include "meta_data_member.hpp"
#include "meta_parsing_instructions.hpp"
#include "indirect_meta_data_member.hpp"
#include "meta_type_descriptor.hpp"
#include "meta_type_conversion.hpp"
#include "meta_value_operation.hpp"
#include "meta_function_call.hpp"
#include "meta_type_reference.hpp"
#include "shared_storage_interface.hpp"
#include "indirect_meta_any.hpp"
#include "indirect_meta_variable_target.hpp"

#include "events.hpp"

namespace engine
{
	template <>
	void reflect<MetaAnyWrapper>()
	{
		engine_meta_type<MetaAnyWrapper>()
			// Added for conversion compatibility.
			.base<MetaAny>()
			
			//.ctor<MetaAny>()
			//.ctor<const MetaAny&>()
			.ctor<MetaAny&&>()

			.conv<MetaAny>()
			//.conv<MetaAny&>()
			//.conv<MetaAny*>()
			.conv<const MetaAny&>()
			//.conv<const MetaAny*>()
		;
	}

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
	void reflect<MetaVariableTarget>()
	{
		engine_meta_type<MetaVariableTarget>()
			.data<&MetaVariableTarget::name>("name"_hs)
			.data<&MetaVariableTarget::scope>("scope"_hs)
			.ctor
			<
				decltype(MetaVariableTarget::name),
				decltype(MetaVariableTarget::scope)
			>()
		;
	}

	template <>
	void reflect<IndirectMetaVariableTarget>()
	{
		engine_meta_type<IndirectMetaVariableTarget>()
			.data<&IndirectMetaVariableTarget::target>("target"_hs)
			.data<&IndirectMetaVariableTarget::variable>("variable"_hs)
		;
	}

	template <>
	void reflect<MetaVariableContext>()
	{
		static_engine_meta_type<MetaVariableContext>()
			.func<static_cast<MetaSymbolID(*)(MetaSymbolID, std::string_view, MetaVariableScope)>(&MetaVariableContext::resolve_path)>("resolve_path"_hs)
			.func<static_cast<MetaSymbolID(*)(MetaSymbolID, MetaSymbolID, MetaVariableScope)>(&MetaVariableContext::resolve_path)>("resolve_path"_hs)
		;
	}

	template <>
	void reflect<MetaDataMember>()
	{
		engine_meta_type<MetaDataMember>()
			.data<&MetaDataMember::type_id>("type_id"_hs)
			.data<&MetaDataMember::data_member_id>("data_member_id"_hs)
			
			//.data<nullptr, &MetaDataMember::get_type>("type"_hs)
			.data<nullptr, &MetaDataMember::has_type>("has_type"_hs)
		;
	}

	template <>
	void reflect<IndirectMetaDataMember>()
	{
		engine_meta_type<IndirectMetaDataMember>()
			.data<&IndirectMetaDataMember::target>("target"_hs)
			.data<&IndirectMetaDataMember::data_member>("data_member"_hs)
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
				const MetaParsingInstructions&,
				std::size_t,
				std::optional<MetaTypeDescriptor::SmallSize>,
				const MetaTypeDescriptorFlags&
			>()

			.ctor
			<
				MetaType, const util::json&,
				const MetaParsingInstructions&,
				//std::size_t,
				std::optional<MetaTypeDescriptor::SmallSize>,
				const MetaTypeDescriptorFlags&
			>()

			.data<&MetaTypeDescriptor::set_type_id, &MetaTypeDescriptor::get_type_id>("type_id"_hs)
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
			.func<static_cast<MetaAny (MetaTypeDescriptor::*)() const>(&MetaTypeDescriptor::operator())>("instance"_hs) // &MetaTypeDescriptor::instance
		;
	}

	template <>
	void reflect<MetaTypeConversion>()
	{
		engine_meta_type<MetaTypeConversion>()
			.data<&MetaTypeConversion::type_id>("type_id"_hs)
			.func<&MetaTypeConversion::get_type>("type"_hs)
			.ctor<decltype(MetaTypeConversion::type_id)>()
		;
	}

	template <>
	void reflect<MetaValueOperation>()
	{
		engine_meta_type<MetaValueOperation>()
			.data<&MetaValueOperation::segments>("segments"_hs)
		;

		engine_meta_type<MetaValueOperation::Segment>()
			.data<&MetaValueOperation::Segment::value>("value"_hs)
			.data<&MetaValueOperation::Segment::operation>("operation"_hs)
		;
	}

	template <>
	void reflect<MetaFunctionCall>()
	{
		engine_meta_type<MetaFunctionCall>()
			.data<&MetaFunctionCall::type_id>("type_id"_hs)
			.data<&MetaFunctionCall::function_id>("function_id"_hs)
			.data<&MetaFunctionCall::arguments>("arguments"_hs)
			.data<&MetaFunctionCall::self>("self"_hs)
		;
	}

	template <>
	void reflect<MetaTypeReference>()
	{
		engine_meta_type<MetaTypeReference>()
			.data<&MetaTypeReference::type_id>("type_id"_hs)
		;
	}

	template <>
	void reflect<SharedStorageInterface>()
	{
		engine_meta_type<SharedStorageInterface>()
			//.func<&SharedStorageInterface::allocate>("allocate"_hs)
			//.func<&SharedStorageInterface::get_storage_as_any>("get_storage"_hs)
			//.func<&SharedStorageInterface::get_storage>("get_storage_ptr"_hs)
		;
	}

	template <>
	void reflect<IndirectMetaAny>()
	{
		engine_meta_type<IndirectMetaAny>()
			.data<nullptr, &IndirectMetaAny::get_type>("type"_hs)
			.data<nullptr, &IndirectMetaAny::get_type_id>("type_id"_hs)
			.data<nullptr, &IndirectMetaAny::get_checksum>("checksum"_hs)

			.func<&IndirectMetaAny::validate_checksum>("validate_checksum"_hs)
			.func<static_cast<MetaAny (IndirectMetaAny::*)(Registry&, Entity) const>(&IndirectMetaAny::get)>("get"_hs)

			.ctor<MetaTypeID, IndirectMetaAny::IndexType, IndirectMetaAny::ChecksumType>()
			.ctor<MetaTypeID, IndirectMetaAny::IndexType>()

			.ctor<MetaType, IndirectMetaAny::IndexType, IndirectMetaAny::ChecksumType>()
			.ctor<MetaType, IndirectMetaAny::IndexType>()

			.ctor<const SharedStorageInterface&, const MetaAny&>()
			.ctor<SharedStorageInterface&, MetaAny&&>()
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
		reflect<MetaAnyWrapper>();
		reflect<MetaVariable>();
		reflect<MetaVariableScope>();
		reflect<MetaVariableTarget>();
		reflect<IndirectMetaVariableTarget>();
		reflect<MetaVariableContext>();
		reflect<MetaDataMember>();
		reflect<IndirectMetaDataMember>();
		reflect<MetaTypeDescriptor>();
		reflect<MetaTypeConversion>();
		reflect<MetaValueOperation>();
		reflect<MetaFunctionCall>();
		reflect<MetaTypeReference>();
		reflect<SharedStorageInterface>();
		reflect<IndirectMetaAny>();

		reflect<ComponentEvent>();
	}
}