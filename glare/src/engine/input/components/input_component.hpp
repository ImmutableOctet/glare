#pragma once

#include <engine/input/types.hpp>

namespace engine
{
	// Indicates which input state may control this entity.
	struct InputComponent
	{
		// Indicates the input-state index this entity is listening for.
		// 
		// NOTE: It is considered illegal to change the value of this
		// field without a new `InputComponent` object via a registry.
		// This includes component updates; re-attach instead.
		InputStateIndex input_index; // const

		inline InputStateIndex get_input_index() const
		{
			return input_index;
		}
	};
}