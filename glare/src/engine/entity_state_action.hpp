#pragma once

#include "types.hpp"

//#include "meta/types.hpp"
#include "meta/meta_type_descriptor.hpp"
#include "meta/meta_description.hpp"

#include <variant>

namespace engine
{
	struct EntityStateTransitionAction
	{
		// The name of the state this `entity` will
		// transition to upon activation of `condition`.
		StringHash state_name;
	};

	struct EntityStateCommandAction
	{
		using CommandContent = MetaTypeDescriptor;

		CommandContent command;
	};

	struct EntityStateUpdateAction
	{
		using Components = MetaDescription;

		Components updated_components;
	};

	using EntityStateAction = std::variant
	<
		EntityStateTransitionAction,
		EntityStateCommandAction,
		EntityStateUpdateAction
	>;
}