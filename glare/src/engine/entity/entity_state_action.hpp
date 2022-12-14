#pragma once

#include "types.hpp"

//#include <engine/meta/types.hpp>
#include <engine/meta/meta_type_descriptor.hpp>
#include <engine/meta/meta_description.hpp>

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