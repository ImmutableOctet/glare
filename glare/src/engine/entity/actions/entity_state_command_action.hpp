#pragma once

#include <engine/meta/meta_type_descriptor.hpp>
#include <engine/entity/entity_descriptor_shared.hpp>

namespace engine
{
	// Stores values to be forwarded to a triggered `Command` type.
	struct EntityStateCommandAction
	{
		//using CommandContent = MetaTypeDescriptor;
		//using CommandContent = std::unique_ptr<MetaTypeDescriptor>;
		using CommandContent = EntityDescriptorShared<MetaTypeDescriptor>;
		
		CommandContent command;
	};
}