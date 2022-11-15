#pragma once

#include <engine/reflection.hpp>

#include "components/state_component.hpp"

namespace engine
{
	class StateSystem;

	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(StateComponent, state_index);

	template <>
	void reflect<StateSystem>()
	{
		reflect<StateComponent>();
	}
}