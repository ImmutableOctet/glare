#pragma once

#include "entity_shared_storage_impl.hpp"

#include "event_trigger_condition.hpp"
#include "entity_thread_description.hpp"
#include "entity_state.hpp"

#include <engine/meta/meta_type_descriptor.hpp>
#include <engine/meta/meta_function_call.hpp>
#include <engine/meta/meta_value_operation.hpp>
#include <engine/meta/indirect_meta_variable_target.hpp>
//#include <engine/meta/meta_property.hpp>

namespace engine
{
	using EntitySharedStorage = EntitySharedStorageImpl
	<
		EntityState,
		EntityThreadDescription,
		EventTriggerCondition,
		MetaTypeDescriptor,
		MetaFunctionCall,
		MetaValueOperation,
		IndirectMetaVariableTarget,

		//MetaProperty,
		//EntityTarget,

		// Added due to most implementations being >16 bytes.
		// (i.e. they're unable to fit locally inside of a `MetaAny`)
		std::string
	>;
}